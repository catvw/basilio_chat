#include "basilio_chat.h++" /* for some reason, this doesn't compile if this
                               header isn't first. I don't know why. */
#include "packet.h++"
#include "../socket/Socket.h++"

#include <iostream>
#include <string>
#include <cstdint>

vanwestco::packet::packet(const packet_size l, const packet_type t, 
                          const packet_bitfield inf) 
        : length(l), type(t), info(inf) {
    payload = new char[length];
}
    
vanwestco::packet::packet(const packet_size l, const packet_type t, 
                          const packet_bitfield inf, const char* pl)
        : length(l), type(t), info(inf) {
    payload = new char[length];
    for (int i = 0; i < length; ++i) { /* copy pl to payload */
        payload[i] = pl[i];
    }
}
    
vanwestco::packet::packet(const packet& pack) 
        : length(pack.length), type(pack.type), info(pack.info) {
    payload = new char[length];
    for (int i = 0; i < length; ++i) {
        payload[i] = pack.payload[i];
    }
}
    
vanwestco::packet::packet(packet&& pack) 
        : length(pack.length), type(pack.type), info(pack.info) {
    payload = pack.payload;
    pack.payload = nullptr;
    pack.length = 0;
    pack.type = packet_type::null_packet;
}

vanwestco::packet_size vanwestco::packet::get_length() const {
    return length;
}

vanwestco::packet_type vanwestco::packet::get_type() const {
    return type;
}

vanwestco::packet_bitfield vanwestco::packet::get_info() const {
    return info;
}

const char* vanwestco::packet::get_payload() const {
    return payload;
}

char& vanwestco::packet::operator[](std::size_t index) {
    return payload[index];
}

bool vanwestco::packet::is_self() const {
    return static_cast<bool>(info & 0b0000'0001);
}

vanwestco::packet::~packet() {
    if (payload != nullptr) {
        delete[] payload;
    }
}
/*----------------------------------------------------------------------------*/
vanwestco::packet vanwestco::read_packet(
        const vanwestco::socket::Socket* sock) {
    unsigned char header[header_length];
    
    /* get a header */
    ssize_t read_bytes   /* XXX */
            = sock->read(reinterpret_cast<char*>(header), header_length);
    
    /* build a packet from the header */
    packet pack((static_cast<packet_size>(header[3]) << 24)
              + (static_cast<packet_size>(header[2]) << 16)
              + (static_cast<packet_size>(header[1]) << 8)
              +  static_cast<packet_size>(header[0]),  /* reassembled length */
                 static_cast<packet_type>(header[4]),  /* packet type */
                 static_cast<packet_bitfield>(header[5])); /* is self */
    pack.length -= header_length;
    
    /* get the payload */
    read_bytes += sock->read(pack.payload, pack.length);
    if (read_bytes < pack.length + header_length) {
        throw vanwestco::basilio_chat::exception("EOF on packet stream");
    }
    
    return pack;
}

void vanwestco::write_packet(const vanwestco::socket::Socket* sock, 
                             const packet* pack) {
    vanwestco::packet_size full_length = header_length + pack->length;
    unsigned char raw_packet[full_length];
    
    /* add payload length to header */
    raw_packet[3] = static_cast<char>(full_length >> 24);
    raw_packet[2] = static_cast<char>((full_length >> 16) & 0xFF);
    raw_packet[1] = static_cast<char>((full_length >> 8) & 0xFF);
    raw_packet[0] = static_cast<char>(full_length & 0xFF);
    
    /* add type to header */
    raw_packet[4] = static_cast<char>(pack->type);
    
    /* add is_self to header */
    raw_packet[5] = pack->info;
    
    /* copy the rest of the packet in */
    for (int i = 0; i < pack->length; ++i) {
        raw_packet[header_length + i] = pack->payload[i];
    }
                /* XXX */
    sock->write(reinterpret_cast<char*>(raw_packet), full_length);
}
