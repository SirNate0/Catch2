
//              Copyright Catch2 Authors
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE.txt or copy at
//        https://www.boost.org/LICENSE_1_0.txt)

// SPDX-License-Identifier: BSL-1.0
#ifndef CATCH_OUTPUT_REDIRECT_HPP_INCLUDED
#define CATCH_OUTPUT_REDIRECT_HPP_INCLUDED

#include <catch2/internal/catch_platform.hpp>
#include <catch2/internal/catch_reusable_string_stream.hpp>
#include <catch2/internal/catch_compiler_capabilities.hpp>
#include <catch2/internal/catch_unique_ptr.hpp>

#include <cassert>
#include <cstdio>
#include <iosfwd>
#include <string>

namespace Catch {

    class OutputRedirectNew {
        bool m_redirectActive = false;
        virtual void activateImpl() = 0;
        virtual void deactivateImpl() = 0;
    public:
        virtual ~OutputRedirectNew(); // = default;

        // TODO: check that redirect is not active before retrieving stdout/stderr?
        virtual std::string getStdout() = 0;
        virtual std::string getStderr() = 0;
        virtual void clearBuffers() = 0;
        void activate() {
            assert( !m_redirectActive && "redirect is already active" );
            activateImpl();
            m_redirectActive = true;
        }
        void deactivate() {
            assert( m_redirectActive && "redirect is not active" );
            deactivateImpl();
            m_redirectActive = false;
        }
    };

    Detail::unique_ptr<OutputRedirectNew> makeOutputRedirect( bool actual );

    class RedirectGuard {
        OutputRedirectNew* m_redirect;
        bool m_activate;
        bool m_moved = false;

    public:
        RedirectGuard( bool activate, OutputRedirectNew& redirectImpl );
        ~RedirectGuard() noexcept( false );

        RedirectGuard( RedirectGuard const& ) = delete;
        RedirectGuard& operator=( RedirectGuard const& ) = delete;

        // C++14 needs move-able guards to return them from functions
        RedirectGuard( RedirectGuard&& rhs ) noexcept;
        RedirectGuard& operator=( RedirectGuard&& rhs ) noexcept;
    };

    RedirectGuard scopedActivate( OutputRedirectNew& redirectImpl );
    RedirectGuard scopedDeactivate( OutputRedirectNew& redirectImpl );

#if defined(CATCH_CONFIG_NEW_CAPTURE)

    // Windows's implementation of std::tmpfile is terrible (it tries
    // to create a file inside system folder, thus requiring elevated
    // privileges for the binary), so we have to use tmpnam(_s) and
    // create the file ourselves there.
    class TempFile {
    public:
        TempFile(TempFile const&) = delete;
        TempFile& operator=(TempFile const&) = delete;
        TempFile(TempFile&&) = delete;
        TempFile& operator=(TempFile&&) = delete;

        TempFile();
        ~TempFile();

        std::FILE* getFile();
        std::string getContents();

    private:
        std::FILE* m_file = nullptr;
    #if defined(_MSC_VER)
        char m_buffer[L_tmpnam] = { 0 };
    #endif
    };


    class OutputRedirect {
    public:
        OutputRedirect(OutputRedirect const&) = delete;
        OutputRedirect& operator=(OutputRedirect const&) = delete;
        OutputRedirect(OutputRedirect&&) = delete;
        OutputRedirect& operator=(OutputRedirect&&) = delete;


        OutputRedirect(std::string& stdout_dest, std::string& stderr_dest);
        ~OutputRedirect();

    private:
        int m_originalStdout = -1;
        int m_originalStderr = -1;
        TempFile m_stdoutFile;
        TempFile m_stderrFile;
        std::string& m_stdoutDest;
        std::string& m_stderrDest;
    };

#endif

} // end namespace Catch

#endif // CATCH_OUTPUT_REDIRECT_HPP_INCLUDED
