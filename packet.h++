/**
 * Packet managing stuff.
 * 
 * @author Charles Van West
 * @version 0
 */

#ifndef BASILIO_CHAT_PACKET_HXX
#define BASILIO_CHAT_PACKET_HXX

#include "../socket/Socket.h++"

#include <cstdint>

namespace vanwestco {

typedef uint32_t packet_size;      /* size of packet */
typedef uint8_t  packet_bitfield;  /* bitfield storage */

const packet_size max_payload_length = 1024;
const packet_size header_length = 8;

enum class packet_type : uint8_t {
    null_packet = 0x00,  /* invalid type, technically */
    join        = 0x01,  /* sent on server join */
/*  disconnect  = 0x02,*//* sent on server disconnect */
    ping        = 0x03,  /* sent to ping client/server */
    name_change = 0x04,  /* indicates username change */
    plaintext   = 0x05,  /* indicates plaintext message */
    start_type  = 0x06,  /* user started typing */
    stop_type   = 0x07,  /* user stopped typing */
    audio       = 0x08,  /* audio data */
    typing_list = 0x09   /* list of people who are currently typing */
};

class packet; /* forward declaration */

/**
 * Reads a packet from the given socket.
 * 
 * @param sock the socket to read
 * 
 * @throws vanwestco::basilio_chat::exception if the end of stream happens
 * @throws vanwestco::socket::SocketException if something happens
 */
packet read_packet(const vanwestco::socket::Socket* sock);

/**
 * Writes a packet to the given socket.
 * 
 * @param sock the socket to write to
 * @param pack the packet to write
 * 
 * @throws vanwestco::socket::SocketException if something happens
 */
void write_packet(const vanwestco::socket::Socket* sock, const packet* pack);

/**
 * What a nice way to represent a packet, eh?
 * 
 * @author Charles Van West
 * @version 0
 */
class packet {
public:
    /**
     * Constructs a packet according length and type and (optionally) bitfield,
     * initializing the internal payload to an empty array of the specified
     * length.
     * 
     * @param l the length of the payload
     * @param t the type of the packet
     * @param inf the info bitfield
     */
    packet(const packet_size l, const packet_type t, 
           const packet_bitfield inf = 0);
    
    /**
     * Constructs a packet in its entirety with a copy of the given payload. The
     * used portion of the payload is from index 0 to index l - 1.
     * 
     * @param l the length of the payload
     * @param t the type of the packet
     * @param inf the info bitfield
     * @param pl the payload
     */
    packet(const packet_size l, const packet_type t, 
           const packet_bitfield inf, const char* pl);
    
    /**
     * Copies a packet. Not sure why you want this at the moment.
     * 
     * @param pack the packet to copy
     */
    packet(const packet& pack); 
    
    /**
     * Moves a packet. The old packet has all fields set to zero or nullptr.
     * 
     * @param pack the packet to move from
     */
    packet(packet&& pack);
    
    /**
     * @return the length of the packet
     */
    packet_size get_length() const;
    
    /**
     * @return the type of the packet
     */
    packet_type get_type() const;
    
    /**
     * @return the info of the packet in bitfield form
     */
    packet_bitfield get_info() const;
    
    /**
     * @return the packet payload
     */
    const char* get_payload() const;
    
    /**
     * @return the character at index <index>
     */
    char& operator[](std::size_t index);
    
    /**
     * @return whether the packet is the client's own
     */
    bool is_self() const;
    
    ~packet();

private:
    friend packet vanwestco::read_packet(const vanwestco::socket::Socket*);
    friend void vanwestco::write_packet(const vanwestco::socket::Socket*,
                                        const packet*);
    packet_size length;
    packet_type type;
    packet_bitfield info; /* currently used as a block bool for is_self */
    char* payload;
};

} /* ~namespace vanwestco */

#endif
