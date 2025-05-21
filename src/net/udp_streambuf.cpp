#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <zg/net/udp_streambuf.hpp>
#include <zg/Logger.hpp>
#include <cerrno>
using namespace zg::net::streams;
udp_streambuf::udp_streambuf(const SocketPair& fd_addr_pair) :
		fd(std::get<0>(fd_addr_pair)), addr(std::get<1>(fd_addr_pair)), gbuffer(4196)
{
	setg(gbuffer.data(), gbuffer.data(), gbuffer.data());
}
udp_streambuf::~udp_streambuf()
{
	sync();
	close();
}
void udp_streambuf::add_received_data(const char* data, size_t size) {
	// Ensure the buffer is large enough to hold the new data
	if (gbuffer.size() < size) {
		gbuffer.resize(size);
	}

	// Copy the received data into the internal buffer
	std::copy(data, data + size, gbuffer.begin());

	// Set the get pointers to make the copied data available for reading:
	// eback() points to the beginning of the buffer (gbuffer.data())
	// gptr() points to the current read position (start reading from the beginning)
	// egptr() points to the end of the valid data in the buffer (gbuffer.data() + size)
	setg(gbuffer.data(), gbuffer.data(), gbuffer.data() + size);

	//zg::Logger::print(zg::Logger::Blank, "Streambuf: Added ", size, " bytes to buffer. Get area set.");
}
int udp_streambuf::underflow()
{
	// If there is already data available in the current get area,
	// just return the next character without trying to refill.
	if (gptr() < egptr())
	{
		// zg::Logger::print(zg::Logger::Blank, "Streambuf: Data available in buffer, returning next char.");
		return traits_type::to_int_type(*gptr());
	}

	// If the get area is empty, attempt to receive the next packet from the socket.
	// We will read up to the size of our internal buffer.
	SocketLength client_len = sizeof(addr); // Need a variable for the length argument
	// Note: For UDP, recvfrom reads one entire datagram. If the buffer is smaller
	// than the packet, the rest of the packet might be lost. If the buffer is larger,
	// recvfrom will only fill up to the packet size.
	long long bytes_received = recvfrom(fd, gbuffer.data(), gbuffer.size(), 0, (struct sockaddr*)&addr, &client_len);

	if (bytes_received > 0)
	{
		// Successfully received data. Make it available in the buffer.
		// eback() points to the beginning of the buffer (gbuffer.data())
		// gptr() points to the current read position (start reading from the beginning)
		// egptr() points to the end of the valid data in the buffer (gbuffer.data() + bytes_received)
		setg(gbuffer.data(), gbuffer.data(), gbuffer.data() + bytes_received);

		// Return the first character of the newly received data
		// zg::Logger::print(zg::Logger::Blank, "Streambuf: Received ", bytes_received, " bytes in underflow. Returning first char.");
		return traits_type::to_int_type(*gptr());
	}
	else if (bytes_received == 0)
	{
		// recvfrom returned 0. For UDP, this might indicate an issue
		// or specific condition, not typically end-of-stream like TCP.
		// Treat as no data available for now.
		// zg::Logger::print(zg::Logger::Blank, "Streambuf: recvfrom returned 0 bytes in underflow.");
		return traits_type::eof(); // Indicate end of available data
	}
	else // bytes_received == -1
	{
		// An error occurred during recvfrom.
		int error_value = errno; // Capture errno immediately
		// Check for specific errors like timeout (EAGAIN/EWOULDBLOCK)
		if (error_value == EAGAIN || error_value == EWOULDBLOCK) {
			// Timeout occurred or socket is non-blocking and no data was ready
			// No data available at this moment.
			// zg::Logger::print(zg::Logger::Blank, "Streambuf: recvfrom timed out or no data available in underflow (EAGAIN/EWOULDBLOCK).");
			return traits_type::eof(); // Indicate no data available
		} else {
			// A different, potentially serious error occurred
			perror("recvfrom failed in underflow"); // Print the system error message
			// Handle other errors appropriately, e.g., log and return EOF or throw
			return traits_type::eof(); // Indicate failure
		}
	}
}
int udp_streambuf::overflow(int c)
{
	// Get the base and current pointers of the put area
	char* pbase_ptr = pbase();
	char* pptr_ptr = pptr();

	// Calculate the number of characters in the put area to send
	size_t num_to_send = pptr_ptr - pbase_ptr;

	if (num_to_send > 0) {
		// Send the data from the put area using sendto
		long long sent = sendto(fd, pbase_ptr, num_to_send, 0, (struct sockaddr*)&addr, addr_len);

		if (sent == -1) {
			// Handle send error
			int error_value = errno;
			zg::Logger::print(zg::Logger::Blank, "sendto failed in overflow. Error code ", error_value, ": ", strerror(error_value));
			// Indicate failure
			return traits_type::eof();
		}

		// Reset the put pointers after sending
		// setp(pbase, epptr) sets the put area base and end.
		// We want to reset the current position (pptr) to the base.
		setp(pbase_ptr, epptr()); // Reset pptr to pbase
	}

	// If a character 'c' was provided (not EOF), add it to the buffer
	if (!traits_type::eq_int_type(c, traits_type::eof())) {
		// Check if there's space for the character
		if (pptr() < epptr()) {
			*pptr() = traits_type::to_char_type(c);
			pbump(1); // Advance the put pointer by 1
			return c; // Indicate success
		} else {
			// Buffer is full, need to send first (which we just did)
			// This case should ideally be handled by the iostream calling overflow
			// before the buffer is completely full, but as a fallback:
			// Try sending the single character immediately if buffer is full
			 long long sent_one = sendto(fd, reinterpret_cast<const char*>(&c), 1, 0, (struct sockaddr*)&addr, addr_len);
			 if (sent_one == -1) {
				 int error_value = errno;
				 zg::Logger::print(zg::Logger::Blank, "sendto failed sending single char in overflow. Error code ", error_value, ": ", strerror(error_value));
				 return traits_type::eof(); // Indicate failure
			 }
			 return c; // Indicate success
		}
	}

	// Indicate success if no character was provided or after sending buffer
	return traits_type::not_eof(c);
}
int udp_streambuf::sync()
{
	// Call overflow with EOF to flush any data in the put buffer
	if (overflow() == traits_type::eof()) {
		return -1; // Indicate failure
	}
	return 0; // Indicate success
}
void udp_streambuf::close()
{
	if (fd >= 0)
	{
#ifdef _WIN32
		::closesocket(fd);
#else
		::close(fd);
#endif
	}
}
