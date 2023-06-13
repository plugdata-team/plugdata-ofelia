#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <vector>
#include <tuple>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using socklen_t = int;

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif



enum ofxMessageType
{
    // Message from plugdata to the ofxOfeliaLua class
    ofx_lua_init = 1,
    ofx_lua_init_sym,
    
    ofx_lua_do_function_s,
    ofx_lua_do_function_sp,
    ofx_lua_do_function_ss,
    ofx_lua_do_function_sf,
    ofx_lua_do_function_sa,
    
    ofx_lua_do_free_function,
    
    ofx_lua_get_var_by_args,
    ofx_lua_set_var_by_args,
    
    ofx_lua_do_string,
    
    pd_outlet_bang,
    pd_outlet_float,
    pd_outlet_symbol,
    pd_outlet_pointer,
    pd_outlet_list,
    pd_outlet_anything,
    
    pd_log,
    
    canvas_realise_dollar,
    canvas_get_dollar_zero,
    canvas_get_name,
    canvas_get_args,
    canvas_set_args,
    canvas_get_dir,
    canvas_make_filename,
    canvas_get_index,
    canvas_get_position,
    canvas_set_position,
    
    pd_send_bang,
    pd_send_float,
    pd_send_symbol,
    pd_send_pointer,
    pd_send_list,
    pd_send_anything,
    
    pd_value_get,
    pd_value_set,
    
    pd_array_get,
    pd_array_set,
    pd_array_get_size,
    pd_array_set_size,
    
    pd_inlet_set_float,
    pd_inlet_set_symbol,
    pd_inlet_set_inlets,
    pd_inlet_set_signal,
    
    pd_get_sys_info
};


class ofxOfeliaStream {
private:
    int socket_;
    int send_port_;
    struct sockaddr_in address_;
    static constexpr size_t buffer_size = 65500;
    
public:
    ofxOfeliaStream() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock." << std::endl;
        }
#endif
        socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_ < 0) {
            std::cerr << "Failed to create socket." << std::endl;
        }
    }

    ~ofxOfeliaStream() {
#ifdef _WIN32
        closesocket(socket_);
        WSACleanup();
#else
        close(socket_);
#endif
    }
    
    
    bool bind(int receive_port, int send_port) {
        send_port_ = send_port;
        address_.sin_family = AF_INET;
        address_.sin_addr.s_addr = htonl(INADDR_ANY);
        address_.sin_port = htons(receive_port);

        if (::bind(socket_, (struct sockaddr*)&address_, sizeof(address_)) < 0) {
            std::cerr << "Failed to bind socket." << std::endl;
            return false;
        }

        return true;
    }
    
    std::string receive(bool blocking = false)
    {
        std::string rcvd;
        if(blocking)
        {
            rcvd = receiveBlocking();
        }
        else
        {
            rcvd = receiveNonBlocking();
        }
        
        return rcvd;
    }


    // Internal function for parsing received messages from stringstream to any number of types
    template <int I, typename Result, typename T, typename... Types>
    static void parse(std::stringstream &istream, Result &result)
    {
        T currentValue;

        // Handle string parsing
        if constexpr (std::is_same<T, std::string>())
        {
            int length;
            istream.read(reinterpret_cast<char *>(&length), sizeof(int));

            auto buffer = std::vector<char>(length);
            istream.read(buffer.data(), length);

            currentValue = T(buffer.data(), length);
        }
        // Handle parsing a list of strings
        else if constexpr (std::is_same<T, std::vector<t_atom>>())
        {
            int listLength;
            istream.read(reinterpret_cast<char *>(&listLength), sizeof(int));

            std::vector<t_atom> atoms(listLength);
            
            for (int i = 0; i < listLength; i++)
            {
                t_atomtype type;
                istream.read(reinterpret_cast<char *>(&type), sizeof(int));
                atoms[i].a_type = type;
                
                if(type == A_FLOAT)
                {
                    istream.read(reinterpret_cast<char *>(&atoms[i].a_w.w_float), sizeof(float));
                }
                if(type == A_SYMBOL)
                {
                    #if FAKE_PD_INTERFACE
                        int length;
                        istream.read(reinterpret_cast<char *>(&length), sizeof(int));
                        
                        auto buffer = std::vector<char>(length);
                        istream.read(buffer.data(), length);                        
                        atoms[i].a_w.w_symbol = std::string(buffer.data(), buffer.size());
                    #else
                        int length;
                        istream.read(reinterpret_cast<char *>(&length), sizeof(int));
                        
                        auto buffer = std::vector<char>(length);
                        istream.read(buffer.data(), length);
                        atoms[i].a_w.w_symbol = gensym(buffer.data());
                    #endif
                }
                if(type == A_POINTER)
                {
                    int value;
                    istream.read(reinterpret_cast<char *>(&value), sizeof(int));
                    atoms[i].a_w.w_gpointer = reinterpret_cast<t_gpointer*>(value);
                }
            }
            
            currentValue = atoms;
        }
        // Read any kind of numeric type, this will work for at least: short, int, long, float, double, bool
        else if constexpr (std::is_arithmetic<T>::value)
        {
            istream.read(reinterpret_cast<char *>(&currentValue), sizeof(T));
        }
        else
        {
            // You're trying to parse an unsupported type from the stream!
            assert(false);
        }

        // Assign value to result tuple
        std::get<I>(result) = currentValue;

        // Don't recurse if Types... is empty
        if constexpr (sizeof...(Types))
        {
            parse<I + 1, Result, Types...>(istream, result);
        }
    }
    
    bool convertAddress(struct sockaddr_in* destAddr) {
    #ifdef _WIN32
        int result = InetPton(AF_INET, "127.0.0.1", &(destAddr->sin_addr));
        return (result == 1);
    #else
        int result = inet_pton(AF_INET, "127.0.0.1", &(destAddr->sin_addr));
        return (result == 1);
    #endif
    }
    
    // Formats message to stringstream and sends it to the other process
    template <typename... Types>
    void sendMessage(Types... args)
    {
        std::stringstream stream;
        writeToStream(stream, args...);

        struct sockaddr_in destAddr;
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(send_port_);
        if (convertAddress(&destAddr) <= 0) {
            std::cerr << "Failed to convert IP address." << std::endl;
            return;
        }

        auto message = stream.str();
        
        ::sendto(socket_, message.c_str(), message.size(), 0,
                        (struct sockaddr*)&destAddr, sizeof(destAddr));
        
    }
    
    // Function for parsing messages
    template <typename... Types>
    static std::tuple<Types...> parseMessage(const std::string &message)
    {
        using Result = std::tuple<Types...>;
        Result result;
        auto istream = std::stringstream(message);
        parse<0, Result, Types...>(istream, result);

        return result;
    }
    
    
private:
    
    // Write any set of arguments to a stringstream for sending over our IPC stream
    template <typename T, typename... Types>
    static void writeToStream(std::stringstream &ostream, T arg, Types... rest)
    {
        // Write a string
        if constexpr (std::is_same<T, std::string>())
        {
            int length = static_cast<int>(arg.length());
            auto* cstr = arg.c_str();
            
            ostream.write(reinterpret_cast<char *>(&length), sizeof(int));
            ostream.write(cstr, length);
        }
        // Write a vector of strings
        else if constexpr (std::is_same<T, std::vector<t_atom>>())
        {
            int size = static_cast<int>(arg.size());
            ostream.write(reinterpret_cast<char *>(&size), sizeof(int));
            
            for (const auto& atom : arg)
            {
                ostream.write(reinterpret_cast<const char *>(&atom.a_type), sizeof(int));
                
                if(atom.a_type == A_FLOAT)
                {
                    ostream.write(reinterpret_cast<const char *>(&atom.a_w.w_float), sizeof(float));
                }
                else if(atom.a_type == A_SYMBOL)
                {
#if FAKE_PD_INTERFACE
                    auto* symbol = atom.a_w.w_symbol.c_str();
#else
                    auto* symbol = atom.a_w.w_symbol->s_name;
#endif
                    int length = strlen(symbol);
                    ostream.write(reinterpret_cast<char *>(&length), sizeof(int));
                    ostream.write(symbol, length);
                }
                else if(atom.a_type == A_POINTER)
                {
                    // TODO: implement this!
                    assert(false);
                }
                else {
                    assert(false);
                }

            }
        }
        else if constexpr (std::is_arithmetic<T>::value || std::is_same<T, ofxMessageType>())
        {
            ostream.write(reinterpret_cast<char *>(&arg), sizeof(T));
        }
        else
        {
            // You're trying to write an unsupported type to the stream!
            assert(false);
        }

        // Check if there are still any arguments left
        if constexpr (sizeof...(Types))
        {
            writeToStream(ostream, rest...);
        }
    }
    


    std::string receiveBlocking() {
        char buffer[buffer_size];
        struct sockaddr_in srcAddr;
        socklen_t addrLen = sizeof(srcAddr);

        int bytesRead = ::recvfrom(socket_, buffer, sizeof(buffer) - 1, 0,
                                       (struct sockaddr*)&srcAddr, &addrLen);
        if (bytesRead > 0) {
            return std::string(buffer, bytesRead);
        }

        return {};
    }

    std::string receiveNonBlocking() {
    #ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(socket_, FIONBIO, &mode);
    #else
        int flags = fcntl(socket_, F_GETFL, 0);
        fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
    #endif

        char buffer[buffer_size];
        struct sockaddr_in srcAddr;
        socklen_t addrLen = sizeof(srcAddr);

        int bytesRead = ::recvfrom(socket_, buffer, sizeof(buffer) - 1, 0,
                                       (struct sockaddr*)&srcAddr, &addrLen);

    #ifdef _WIN32
        mode = 0;
        ioctlsocket(socket_, FIONBIO, &mode);
    #else
        flags = fcntl(socket_, F_GETFL, 0);
        flags &= ~O_NONBLOCK;
        fcntl(socket_, F_SETFL, flags);
    #endif

        if (bytesRead > 0) {            
            return std::string(buffer, bytesRead);
        }

        return {};
    }
    
};

