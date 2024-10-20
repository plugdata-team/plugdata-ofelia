#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <memory>
#include <sstream>
#include <tuple>

#ifdef PD
#include <m_pd.h>
#else
#include "../Runner/ofxPdInterface.h"
#endif

#include "TcpSocket.h"

#ifdef PD
    using SocketType = TcpServerSocket;
#else
    using SocketType = TcpClientSocket;
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
    ofx_lua_audio_block,
    
    ofx_quit,
        
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
    
    pd_audio_block,
    
    pd_get_sys_info
};

#ifdef _WIN32
using socklen_t = int;
#endif

class ofxOfeliaStream {
    
public:
        
    void initialise(int recv_port, int snd_port);

    bool bind();
    
    void quit();
    
    void setBlocking(bool blocking)
    {
        if(receiveSocket) receiveSocket->setBlocking(blocking);
    }

    std::string receive();
    void send(std::string toSend);

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
        // Handle parsing a list of atoms
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
                    #ifndef PD
                        int length;
                        istream.read(reinterpret_cast<char *>(&length), sizeof(int));
                        
                        auto buffer = std::vector<char>(length);
                        istream.read(buffer.data(), length);                        
                        atoms[i].a_w.w_symbol = std::string(buffer.data(), buffer.size());
                    #else
                        int length;
                        istream.read(reinterpret_cast<char *>(&length), sizeof(int));
                        
                        auto buffer = std::vector<char>(length + 1);
                        istream.read(buffer.data(), length);
                        buffer[length - 1] = '\0';
                        atoms[i].a_w.w_symbol = gensym(buffer.data());
                    #endif
                }
                if(type == A_POINTER)
                {
                    intptr_t value;
                    istream.read(reinterpret_cast<char *>(&value), sizeof(intptr_t));
                    atoms[i].a_w.w_gpointer = reinterpret_cast<t_gpointer*>(value);
                }
            }
            
            currentValue = atoms;
        }
        else if constexpr (std::is_same<T, std::vector<float>>())
        {
            int size;
            istream.read(reinterpret_cast<char *>(&size), sizeof(int));

            auto buffer = std::vector<float>(size);
            istream.read(reinterpret_cast<char*>(buffer.data()), size * sizeof(float));

            currentValue = buffer;
        }
        else if constexpr (std::is_same<T, std::vector<std::vector<float>>>())
        {
            int outerSize;
            istream.read(reinterpret_cast<char*>(&outerSize), sizeof(int));

            currentValue.clear(); // Clear the current value to populate it with new data
            
            for (int i = 0; i < outerSize; ++i)
            {
                int innerSize;
                istream.read(reinterpret_cast<char*>(&innerSize), sizeof(int));

                std::vector<float> innerVector(innerSize);
                istream.read(reinterpret_cast<char*>(innerVector.data()), innerSize * sizeof(float));

                currentValue.push_back(innerVector);
            }
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
        
    // Formats message to stringstream and sends it to the other process
    template <typename... Types>
    void sendMessage(Types... args)
    {
        std::stringstream stream;
        writeToStream(stream, args...);

        send(stream.str());        
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
#ifndef PD
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
        else if constexpr (std::is_same<T, std::vector<float>>())
        {
            int size = static_cast<int>(arg.size());
            auto* floatPtr = arg.data();
            
            ostream.write(reinterpret_cast<char *>(&size), sizeof(int));
            ostream.write(floatPtr, size * sizeof(float));
        }
        else if constexpr (std::is_same<T, std::vector<std::vector<float>>>())
        {
            int outerSize = static_cast<int>(arg.size());
            ostream.write(reinterpret_cast<const char*>(&outerSize), sizeof(int));

            for (const auto& innerVector : arg)
            {
                int innerSize = static_cast<int>(innerVector.size());
                ostream.write(reinterpret_cast<const char*>(&innerSize), sizeof(int));

                ostream.write(reinterpret_cast<const char*>(innerVector.data()), innerSize * sizeof(float));
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
    
        
    std::unique_ptr<SocketType> sendSocket;
    std::unique_ptr<SocketType> receiveSocket;
    
    static constexpr size_t buffer_size = 65500;
    std::vector<char> buffer = std::vector<char>(buffer_size, 0);
};

