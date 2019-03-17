// Copyright (c) 2019 Tommy Nguyen. Distributed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef PANINI__PANINI_HPP_
#define PANINI__PANINI_HPP_

#include <iterator>
#include <string>
#include <string_view>
#include <utility>

namespace panini
{
    using czstring = const char*;

    enum class State {
        Undetermined,
        Key,
        Section,
        SectionBegin,
        SectionEnd,
        Value,
        ValueBegin,
        Error,
    };

    namespace detail
    {
        struct Context {
            int32_t line = 0;
            State state = State::Undetermined;
            czstring section = nullptr;
            czstring section_end = nullptr;
            czstring key = nullptr;
            czstring key_end = nullptr;
            czstring value = nullptr;
            czstring value_end = nullptr;
        };

        constexpr auto make_string_view(czstring begin, czstring end)
        {
            return std::string_view(begin, std::distance(begin, end));
        }
    }  // namespace detail

    template <typename F>
    void parse(czstring c, F&& callback)
    {
        std::string_view error;
        detail::Context ctx;

        for (;; ++c) {
            switch (*c) {
                case '\0':
                case '\n':
                case '\r':
                    switch (ctx.state) {
                        case State::Key:
                            error = "Unexpected end of key";
                            goto error;

                        case State::Section:
                        case State::SectionBegin:
                            error = "Unexpected end of section";
                            goto error;

                        case State::ValueBegin:
                            ctx.value = c;
                            [[fallthrough]];

                        case State::Value:
                            ctx.value_end = c;
                            callback(  //
                                State::Value,
                                detail::make_string_view(
                                    ctx.section, ctx.section_end),
                                detail::make_string_view(ctx.key, ctx.key_end),
                                detail::make_string_view(
                                    ctx.value, ctx.value_end));
                            [[fallthrough]];

                        case State::Undetermined:
                        case State::SectionEnd:
                            ctx = {
                                ctx.line + 1,
                                State::Undetermined,
                                ctx.section,
                                ctx.section_end,
                            };
                            break;

                        case State::Error:
                            break;
                    }
                    if (*c == '\0') {
                        return;
                    }
                    break;

                case '\t':
                case ' ':
                    break;

                case ';':
                    if (ctx.state == State::Undetermined ||
                        ctx.state == State::Value) {
                        while (*(++c) != '\n' && *c != '\r') {
                            if (*c == '\0') {
                                return;
                            }
                        }

                        ctx = {
                            ctx.line + 1,
                            State::Undetermined,
                            ctx.section,
                            ctx.section_end,
                        };
                    }
                    break;

                case '=':
                    if (ctx.state != State::Key) {
                        error = "Expected key before '='";
                        goto error;
                    }
                    ctx.state = State::ValueBegin;
                    break;

                case '[':
                    if (ctx.state == State::Undetermined) {
                        ctx = {ctx.line, State::SectionBegin};
                        break;
                    }
                    [[fallthrough]];

                case ']':
                    if (ctx.state == State::Section) {
                        ctx.state = State::SectionEnd;
                        break;
                    }
                    [[fallthrough]];

                default:
                    switch (ctx.state) {
                        case State::Undetermined:
                            ctx.state = State::Key;
                            ctx.key = c;
                            break;
                        case State::Key:
                            ctx.key_end = c + 1;
                            break;
                        case State::Section:
                            ctx.section_end = c + 1;
                            break;
                        case State::SectionBegin:
                            ctx.state = State::Section;
                            ctx.section = c;
                            break;
                        case State::SectionEnd:
                            break;
                        case State::Value:
                            ctx.value_end = c + 1;
                            break;
                        case State::ValueBegin:
                            ctx.state = State::Value;
                            ctx.value = c;
                            break;
                        case State::Error:
                            break;
                    }
                    break;
            }
        }

        return;

    error:
        const auto line = std::to_string(ctx.line + 1);
        callback(State::Error, line, error, {});
    }
}  // namespace panini

#endif
