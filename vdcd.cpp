#include <algorithm>
#include <chrono>
#include <ostream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/range.hpp>
#include <json/json.h>

#include "logging.hpp"
#include "manager.hpp"
#include "vdcd.hpp"

using namespace std;
using namespace asio;
using namespace ip;

namespace sc {

	static Logger logger( "vdcd" );

	static PropertyKey const hostProperty( "host" );
	static PropertyKey const portProperty( "port", 8999 );

	Vdcd::Vdcd( Manager& manager, string id, PropertyNode const& properties )
		: Component( "vdcd", move( id ) )
		, manager_( manager )
		, host_( properties[ hostProperty ].as< string >() )
		, port_( properties[ portProperty ].as< string >() )
		, socket_( manager_.service() )
	{
		Json::StreamWriterBuilder builder;
		builder[ "indentation" ] = "";
		jsonWriter_.reset( builder.newStreamWriter() );

		manager_.readyEvent().subscribe( [this] { connect(); }, true );
	}

	void Vdcd::add( VdcdDevice* device )
	{
		devices_.push_back( device );
	}

	void Vdcd::remove( VdcdDevice* device )
	{
		devices_.erase( std::remove( devices_.begin(), devices_.end(), device ), devices_.end() );
	}

	void Vdcd::connect()
	{
		logger.info( "connecting to vdcd at ", host_, ":", port_ );

		tcp::resolver resolver( manager_.service() );
		async_connect(
				socket_, resolver.resolve( tcp::resolver::query( host_, port_ ) ),
				[this] ( error_code ec, tcp::resolver::iterator ) {
					if ( handleError( ec ) ) {
						return;
					}

					sendInitMessage();
				} );
	}

	void Vdcd::reconnect()
	{
		socket_.close();
		incoming_.consume( incoming_.size() );
		outgoing_.consume( outgoing_.size() );
		retryConnect();
	}

	void Vdcd::retryConnect()
	{
		int retryMs = 1000;

		auto timer = make_shared< steady_timer >( manager_.service() );
		timer->expires_from_now( chrono::milliseconds( retryMs ) );
		timer->async_wait( [this, timer] ( error_code ec ) {
			if ( ec.value() == (int) errc::operation_canceled ) {
				return;
			}
			connect();
		} );
	}

	void Vdcd::sendInitMessage()
	{
		Json::Value json( Json::arrayValue );
		for ( size_t i = 0 ; i < devices_.size() ; ++i ) {
			auto const& device = *devices_[ i ];
			Json::Value jsonDevice( Json::objectValue );
			jsonDevice[ "message" ] = "init";
			jsonDevice[ "name" ] = device.name();
			jsonDevice[ "tag" ] = string( to_string( i ) );
			jsonDevice[ "uniqueid" ] = device.dsuid();
			jsonDevice[ "group" ] = device.group();
			jsonDevice[ "sync" ] = true;
			if ( !device.outputType().empty() ) {
				jsonDevice[ "output" ] = device.outputType();
				jsonDevice[ "dimmable" ] = device.dimmable();
			}
			json.append( jsonDevice );
		}
		send( json, [this] { receiveInitResponse(); } );
	}

	void Vdcd::receiveInitResponse()
	{
		receive( [this]( Json::Value const& json ) {
			if ( json[ "message" ].asString() != "status" || json[ "status" ].asString() != "ok" ) {
				logger.error( "message from vdcd: ", json[ "errormessage" ].asString() );
				reconnect();
				return;
			}

			logger.info( "connection to vdcd established" );

			receive( [this]( Json::Value const& json ) { parse( json ); } );
		}, false );
	}

	void Vdcd::sendChannelMessage( size_t device )
	{
		Json::Value json( Json::objectValue );
		json[ "message" ] = "channel";
		json[ "tag" ] = string( to_string( device ) );
		json[ "index" ] = "0";
		json[ "value" ] = devices_[ device ]->get();
		send( json, [this, device] {
			sendSyncedMessage( device );
		} );
	}

	void Vdcd::sendSyncedMessage( size_t device )
	{
		Json::Value json( Json::objectValue );
		json[ "message" ] = "synced";
		json[ "tag" ] = string( to_string( device ) );
		send( json, []{} );
	}

	void Vdcd::send( Json::Value const& json, function< void () > const& handler )
	{
		auto start = outgoing_.size();
		ostream os( &outgoing_ );
		jsonWriter_->write( json, &os );
		os << '\n' << flush;

		logger.debug(
				"<<< ", boost::make_iterator_range(
					next( buffers_begin( outgoing_.data() ), start ),
					next( buffers_begin( outgoing_.data() ), outgoing_.size() - 1 ) ) );

		/*
		Json::FastWriter writer;
		string string = writer.write( json );

		logger.debug( "<<< ", string );

		ostream os( &outgoing_ );
		os << string << '\n';
		*/
		async_write(
				socket_, outgoing_,
				[this, handler] ( error_code ec, size_t written ) {
					if ( handleError( ec ) ) {
						return;
					}

					handler();
				} );
	}

	void Vdcd::receive( function< void ( Json::Value const& ) > const& handler, bool repeat )
	{
		async_read_until(
				socket_, incoming_, '\n',
				[this, handler, repeat]( error_code ec, size_t ) {
					if ( handleError( ec ) ) {
						return;
					}

					auto begin = buffers_begin( incoming_.data() );
					auto end = buffers_end( incoming_.data() );

					auto first = begin;
					decltype( first ) last;
					while ( ( last = find( first, end, '\n' ) ) != end ) {
						string message( first, last );
						logger.debug( ">>> ", message );

						Json::Value json;
						if ( !Json::Reader().parse( message, json ) ) {
							logger.error( "couldn't read from vdcd: invalid JSON message" );
							reconnect();
							return;
						}
						handler( json );

						first = last + 1;
					}
					incoming_.consume( distance( begin, first ) );

					if ( repeat ) {
						receive( handler, repeat );
					}
				} );
	}

	void Vdcd::parse( Json::Value const& json )
	{
		if ( json[ "message" ].asString() == "channel" ) {
			size_t device = boost::lexical_cast< size_t >( json[ "tag" ].asString() );
			double value = json[ "value" ].asDouble();
			devices_[ device ]->set( value );
		}
		else if ( json[ "message" ].asString() == "sync" ) {
			size_t device = boost::lexical_cast< size_t >( json[ "tag" ].asString() );
			sendChannelMessage( device );
		}
		else {
			auto message = json[ "message" ];
			logger.error(
					message.isNull() ? "unknown" : "unexpected", " message from vdcd",
					message.isNull() ? "" : ": ", message.asString() );
			socket_.close();
		}
	}

	bool Vdcd::handleError( error_code const& ec )
	{
		if ( ec ) {
			logger.error( "socket error in vdcd client: ", ec.message() );
			reconnect();
			return true;
		}
		return false;
	}

	VdcdDevice::VdcdDevice(
			Manager& manager, string const& requester, string const& vdcdId, string const& dsuid, int group,
			string const& outputType, bool dimmable, SetterFunction const& setterFunction )
		: vdcd_( manager.get< Vdcd >( requester, vdcdId ) )
		, name_( requester )
		, dsuid_( dsuid )
		, group_( group )
		, outputType_( outputType )
		, dimmable_( dimmable )
		, setterFunction_( setterFunction )
	{
		vdcd_.add( this );
	}

	VdcdDevice::~VdcdDevice()
	{
		vdcd_.remove( this );
	}

	void VdcdDevice::set( double value )
	{
		if ( value_ != value ) {
			value_ = value;
			if ( setterFunction_ ) {
				setterFunction_( value_ );
			}
		}
	}

	__attribute__(( unused )) static ComponentRegistry< Vdcd > registry( "vdcd" );

} // namespace sc
