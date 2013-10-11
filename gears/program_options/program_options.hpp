// The MIT License (MIT)

// Copyright (c) 2012-2013 Danny Y., Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef GEARS_PROGRAM_OPTIONS_CLASS_HPP
#define GEARS_PROGRAM_OPTIONS_CLASS_HPP

#include <map>
#include "details.hpp"
#include "arg.hpp"

namespace gears {
class program_options {
private:
    std::map<std::string, arg> args;
    std::string program_name;
    std::string program_usage;

    void parse_short_option(const std::string& option, int& i, char* argv[]) {
        for(auto&& arg : args) {

            auto pos = option.find(arg.second.short_name);

            // Single flag or option with value
            if(option.size() == 2) {
                if(pos != std::string::npos) {
                    if(arg.second.is_value()) {
                        int counter = i + 1;

                        // next arg isn't a flag or option
                        // interspersing values with comma separation
                        if(argv[counter] != nullptr && argv[counter][0] != '-') {
                            arg.second.value.append(argv[counter++]);
                        }

                        while(argv[counter] != nullptr && argv[counter][0] != '-') {                            
                            // append option value to arg value
                            arg.second.value.push_back(' ');
                            arg.second.value.append(argv[counter++]);
                        }

                        i = counter - 1;
                        break;
                    }

                    // regular flag

                    arg.second.active = true;
                }
            }

            // Grouped character or grouped option
            else if(option.size() > 2) {

                // Grouped option
                if(arg.second.is_value() && option[1] == arg.second.short_name) {
                    arg.second.value.append(std::begin(option) + 2, std::end(option));
                    arg.second.active = true;
                    break;
                }

                // regular flag with grouped characters                
                arg.second.active = pos != std::string::npos;
            }
            else {
                // Seems to be neither
                break; 
            }
        }
    }
public:
    program_options() = default;

    program_options(std::initializer_list<arg> l) {
        for(auto&& i : l) {
            args.insert(std::make_pair("--" + i.name, i));
        }
    }

    arg& add(const std::string& name) {
        auto p = args.emplace(std::piecewise_construct, 
                              std::forward_as_tuple("--" + name), 
                              std::forward_as_tuple(name));
        return (p.first)->second;
    }

    void parse(int argc, char* argv[]) noexcept {
        program_name = argv[0];
        std::string current;
        for(int i = 1; i < argc; ++i) {
            current = argv[i];

            // Stop parsing if it's --
            if(current == "--")
                break;

            // current arg has equal sign            
            auto pos = current.find('=');
            if(pos != std::string::npos) {
                auto it = args.find(current.substr(0, pos));

                // make sure the argument is valid
                if(it != args.end()) {
                    it->second.value.append(current.substr(pos + 1));
                }

                continue;
            }

            auto it = args.find(current);

            if(it != args.end()) {

                // value options
                if(it->second.is_value()) {
                    int counter = i + 1;

                    // next arg isn't a flag or value
                    // interspersing values with comma separation
                    if(argv[counter] != nullptr && argv[counter][0] != '-') {
                        it->second.value.append(argv[counter++]);
                    }

                    while(argv[counter] != nullptr && argv[counter][0] != '-') {
                        // append value
                        it->second.value.push_back(' ');
                        it->second.value.append(argv[counter++]);
                    }

                    i = counter - 1;
                }

                // regular flag
                
                it->second.active = true;
                continue;
            }

            // short options found
            if(current.front() == '-') {
                parse_short_option(current, i, argv);
            }
        }
    }

    bool is_active(const std::string& name) const noexcept {
        auto it = args.find("--" + name);
        if(it != args.end()) {
            return it->second.active;
        }
        return false;
    }

    void usage(std::string str) noexcept {
        program_usage = std::move(str);
    }

    std::string usage() const noexcept {
        return "usage: " + program_name + ' ' + program_usage;
    }

    template<typename T = std::string>
    T get(const std::string& str) const {
        auto it = args.find("--" + str);
        if(it != args.end()) {
            if(it->second.is_value())
                return detail::lexical_cast<T>(it->second.value);
        }

        throw detail::arg_not_found(str + " is not an option argument");
    }

    template<typename T>
    T get_list(const std::string& str) const {
        auto it = args.find("--" + str);
        if(it != args.end()) {
            if(it->second.is_value()) {
                T result;
                std::istringstream ss(it->second.value);
                typename T::value_type x;
                while(ss >> x) {
                    detail::insert_impl(result, x);
                }

                return result;
            }
        }

        throw detail::arg_not_found(str + " is not an option argument");
    }

    #ifndef GEARS_NO_IOSTREAM

    template<typename Elem, typename Traits>
    friend auto operator<<(std::basic_ostream<Elem, Traits>& out, const program_options& po) -> decltype(out) {
        out << "usage: " << po.program_name << ' ' << po.program_usage << "\n\n";

        for(auto&& arg : po.args) {
            out << arg.second;            
        }

        return out;
    }

    #endif // GEARS_NO_IOSTREAM
};
} // gears

#endif // GEARS_PROGRAM_OPTIONS_CLASS_HPP