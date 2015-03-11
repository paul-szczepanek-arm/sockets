/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright 2015 ARM Holdings PLC
 */
#ifndef MBED_ASOCKET_H
#define MBED_ASOCKET_H


#include <mbed.h>
#include <stddef.h>
#include <stdint.h>
#include "socket_types.h"

#include "CThunk.h"

#include "SocketAddr.h"

/**
 * \brief aSocket implements most of the interfaces required for sockets.
 * aSocket is a pure virtual class; it should never be instantiated directly, but it provides
 * common functionality for derived classes.
 */
class aSocket {
protected:
	/**
	 * aSocket constructor
	 * Initializes the aSocket object.  Initializes the underlying struct socket.  Does not instantiate
	 * an underlying network stack socket.
	 * Since it is somewhat awkward to provide the network stack, a future change will provide
	 * a way to pass the network interface to the socket constructor, which will extract the stack from
	 * the interface.
	 * @param[in] stack The network stack to use for this socket.
	 */
    aSocket(const socket_stack_t stack);
    /**
     * aSocket destructor
     * Frees the underlying socket implementation.
     */
    virtual ~aSocket();
public:
    /**
     * Extract the current event from the C socket implementation
     * @return an event structure containing an event type and other potentially relevant information
     */
    struct socket_event *getEvent(); // TODO: (CThunk upgrade/Alpha3)
    /**
     * Start the process of resolving a domain name.
     * If the input is a text IP address, an event is queued immediately; otherwise, onDNS is
     * queued as soon as DNS is resolved.
     * The socket must have been opened before resolve is called
     * @param[in] address The domain name to resolve
     * @param[in] onDNS The handler to call when the name is resolved
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    socket_error_t resolve(const char* address, handler_t onDNS);
    /**
     * Open the socket.
     * Instantiates and initializes the underlying socket. Receive is started immediately after
     * the socket is opened.
     * @param[in] af Address family (SOCKET_AF_INET4 or SOCKET_AF_INET6), currently only IPv4 is supported
     * @param[in] pf Protocol family (SOCKET_DGRAM or SOCKET_STREAM)
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t open(const socket_address_family_t af, const socket_proto_family_t pf);
    /**
     * Binds the socket's local address and IP.
     * 0.0.0.0 is accepted as a local address if only the port is meant to be bound.
     * The behaviour of bind("0.0.0.0",...) is undefined where two or more stacks are in use.
     *
     * @param[in] address The string representation of the address to bind
     * @param[in] port The local port to bind
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t bind(const char *address, const uint16_t port);
    /**
     * bind(const SocketAddr *, const uint16_t) is the same as bind(const char *, const uint16_t),
     * except that the address passed in is a SocketAddr.
     * @param[in] address The address to bind
     * @param[in] port The local port to bind
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t bind(const SocketAddr *address, const uint16_t port);
    /**
     * Set the error handler.
     * Errors are ignored if onError is not set.
     * @param[in] onError
     */
    virtual void setOnError(handler_t onError);
    /**
     * Set the received data handler
     * Received data is queued until it is read using recv or recv_from.
     * @param[in] onReadable the handler to use for receive events
     */
    virtual void setOnReadable(handler_t onReadable);
    /**
     * Receive a message
     * @param[out] buf The buffer to fill
     * @param[in,out] len A pointer to the size of the receive buffer.  Sets the maximum number of bytes
     * to read but is updated with the actual number of bytes copied on success.  len is not changed on
     * failure
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t recv(void * buf, size_t *len);
    /**
     * Receive a message with the sender address and port
     * This API is not valid for SOCK_STREAM
     * @param[out] buf The buffer to fill
     * @param[in,out] len A pointer to the size of the receive buffer.  Sets the maximum number of bytes
     * to read but is updated with the actual number of bytes copied on success.  len is not changed on
     * failure
     * @param[out] remote_addr Pointer to an address structure to fill with the sender address
     * @param[out] remote_port Pointer to a uint16_t to fill with the sender port
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t recv_from(void * buf, size_t *len, SocketAddr *remote_addr, uint16_t *remote_port);
    /**
     * Set the onSent handler.
     * The exact moment this handler is called varies from implementation to implementation.
     * On LwIP, onSent is called when the remote host ACK's data in TCP sockets, or when the message enters
     * the network stack in UDP sockets.
     * @param[in] onSent The handler to call when a send completes
     */
    virtual void setOnSent(handler_t onSent);
    /**
     * Send a message
     * Sends a message over an open connection.  This call is valid for UDP sockets, provided that connect()
     * has been called.
     * @param[in] buf The payload to send
     * @param[in] len The size of the payload
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t send(const void * buf, const size_t len);
    /**
     * Send a message to a specific address and port
     * This API is not valid for SOCK_STREAM
     * @param[in] buf The payload to send
     * @param[in] len The size of the payload
     * @param[in] address The address to send to
     * @param[in] port The remote port to send to
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t send_to(const void * buf, const size_t len, const SocketAddr *remote_addr, uint16_t remote_port);
    /**
     * Shuts down a socket.
     * Sending and receiving are no longer possible after close() is called.
     * The socket is not deallocated on close.  A socket must not be reopened, it should be
     * destroyed (either with delete, or by going out of scope) after calling close.
     * Calling open on a closed socket can result in a memory leak.
     * @return SOCKET_ERROR_NONE on success, or an error code on failure
     */
    virtual socket_error_t close();

#if 0 // not implemented yet
    static long ntohl(long);
    static short ntohs(short);
    static long long ntohll(long long);
#endif

protected:
    /**
     * The internal event handler
     * @param[in] ev The event to handle
     */
    virtual void _eventHandler(struct socket_event *ev);
    /**
     * Error checking utility
     * Generates an event on error, does nothing on SOCKET_ERROR_NONE
     * @param[in] err the error code to check
     * @return false if err is SOCKET_ERROR_NONE, true otherwise
     */
    bool error_check(socket_error_t err);

protected:
    handler_t _onDNS;
    handler_t _onError;
    handler_t _onReadable;
    handler_t _onSent;

    CThunk<aSocket> _irq;
    struct socket _socket;
private:
    socket_event_t *_event; // TODO: (CThunk upgrade/Alpha3)
    /**
     * Internal event handler.
     * @param[in] arg
     */
    void _nvEventHandler(void * arg);
};


#endif // MBED_ASOCKET_H
