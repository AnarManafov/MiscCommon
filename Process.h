/************************************************************************/
/**
 * @file Process.h
 * @brief This header contains a subset of helpers for Process, Daemon and PID file operations.
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-04-12
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROCESS_H_
#define PROCESS_H_

// API
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
// STD
#include <fstream>
#include <stdexcept>
#include <iterator>
#include <memory>
// POSIX regexp
#include <regex.h>
// MiscCommon
#include "def.h"
#include "ErrorCode.h"
#include "MiscUtils.h"
#include "CustomIterator.h"
#include "stlx.h"

namespace MiscCommon
{
    /**
     *
     * @brief The function checks, whether the process which corresponds to the given \b _PID can be found.
     * @param[in] _PID - a process ID to look for.
     * @return \b true when the process is found, otherwise return value is \b false.
     * @note This function will not be able to check existence of a zombie process
     *
     */
    inline bool IsProcessExist( pid_t _PID )
    {
        return !( ::kill( _PID, 0 ) == -1 && errno == ESRCH );
    }
    /**
     *
     * @brief A PID-file helper
     *
     */
    class CPIDFile
    {
        public:
            CPIDFile( const std::string &_FileName, pid_t _PID ): m_FileName( _FileName )
            {
                if( !_FileName.empty() && _PID > 0 )
                {
                    // Preventing to start a second "instance" if the pidfile references to the running process
                    const pid_t pid = GetPIDFromFile( m_FileName );
                    if( pid > 0 && IsProcessExist( pid ) )
                    {
                        // We don't want to unlink this file
                        m_FileName.clear();
                        throw std::runtime_error( "Error creating pidfile. The process corresponding to pidfile \"" + _FileName + "\" is still running" );
                    }

                    // Wrtiting new pidfile
                    std::ofstream f( m_FileName.c_str() );
                    if( !f. is_open() )
                        throw std::runtime_error( "can't create PID file: " + m_FileName );

                    f << _PID;
                }
                else
                    m_FileName.clear();
            }

            ~CPIDFile()
            {
                if( !m_FileName.empty() )
                    ::unlink( m_FileName.c_str() );
            }

            static pid_t GetPIDFromFile( const std::string &_FileName )
            {
                std::ifstream f( _FileName.c_str() );
                if( !f. is_open() )
                    return 0;

                pid_t pid( 0 );
                f >> pid;

                return pid;
            }

        private:
            std::string m_FileName;
    };
    /**
     *
     * @brief This class is used to quarry a list of currently running processes
     * @note Usage: In the example container pids will be containing pids of currently running processes
     * @code
     CProcList::ProcContainer_t pids;
     CProcList::GetProcList( &pids );
     * @endcode
     *
     */
    class CProcList
    {
        public:
            typedef std::set<pid_t> ProcContainer_t;

        public:
            static void GetProcList( ProcContainer_t *_Procs )
            {
                if( !_Procs )
                    throw std::invalid_argument( "CProcList::GetProcList: Input container is NULL" );

                _Procs->clear();

                struct dirent **namelist;
                // scanning the "/proc" filesystem
                int n = scandir( "/proc", &namelist, CheckDigit, alphasort );

                if( -1 == n )
                    throw system_error( "CProcList::GetProcList exception" );
                if( 0 == n )
                    return; // there were no files

                for( int i = 0; i < n; ++i )
                {
                    std::stringstream ss( namelist[i]->d_name );
                    pid_t pid;
                    ss >> pid;
                    _Procs->insert( pid );
                    free( namelist[i] );
                }

                free( namelist );
            }

        private:


#ifdef __APPLE__
            static int CheckDigit( struct dirent* _d )
#else
            static int CheckDigit( const struct dirent* _d )
#endif
            {
                const std::string sName( _d->d_name );
                // Checking whether file name has all digits
                return ( sName.end() == std::find_if( sName.begin(), sName.end(), std::not1( IsDigit() ) ) );
            }
    };
    /**
     *
     * @brief This class helps to retrieve process's information from /proc/\<pid\>/status
     * @note Usage:
     * @code
     CProcStatus p;
     p.Open( 8007 );
     cout << "Name" << p.GetValue( "Name" ) << endl;
     cout << "PPid" << p.GetValue( "PPid" ) << endl;
     p.Open( 1 );
     cout << "Name" << p.GetValue( "Name" ) << endl;
     cout << "PPid" << p.GetValue( "PPid" ) << endl;
     * @endcode
     *
     */
// TODO: need a new algorithms for a longer app names retrieval
    class CProcStatus
    {
            typedef std::auto_ptr<std::ifstream> ifstream_ptr;
            typedef std::map<std::string, std::string> keyvalue_t;
        public:
            CProcStatus()
            {
                // Preparing regular expression pattern
                regcomp( &m_re, "(.*):(.*)", REG_EXTENDED );
            }
            ~CProcStatus()
            {
                regfree( &m_re );
            }
            void Open( pid_t _PId )
            {
                m_values.clear();
                if( m_f.get() )
                    m_f->close();

                std::stringstream ss;
                ss
                        << "/proc/"
                        << _PId
                        << "/status";
                m_f = ifstream_ptr( new std::ifstream( ss.str().c_str() ) );
                // create reader objects
                // HACK: the extra set of parenthesis (the last argument of vector's ctor) is required (for gcc 4.1+)
                //      StringVector_t vec( custom_istream_iterator<std::string>(*m_f), (custom_istream_iterator<std::string>()) );
                // or
                // custom_istream_iterator<std::string> in_begin(*m_f);
                //      custom_istream_iterator<std::string> in_end;
                //      StringVector_t vec( in_begin, in_end );
                // the last method for gcc 3.2+
                // , because
                // the compiler is very aggressive in identifying function declarations and will identify the
                // definition of vec as forward declaration of a function accepting two istream_iterator parameters
                // and returning a vector of integers
                custom_istream_iterator<std::string> in_begin( *m_f );
                custom_istream_iterator<std::string> in_end;
                StringVector_t vec( in_begin, in_end );

                for_each( vec.begin(), vec.end(),
                          std::bind1st( MiscCommon::stlx::mem_fun( &CProcStatus::_Parser ), this )
                        );
            }
            std::string GetValue( const std::string &_KeyName ) const
            {
                // We want to be case insensitive
                std::string sKey( _KeyName );
                to_lower( sKey );

                keyvalue_t::const_iterator iter = m_values.find( sKey );
                return( m_values.end() == iter ? std::string() : iter->second );
            }

        private:
            bool _Parser( const std::string &_sVal )
            {
                regmatch_t PMatch[3];
                if( 0 != regexec( &m_re, _sVal.c_str(), 3, PMatch, 0 ) )
                    return false;
                std::string sKey( _sVal.c_str() + PMatch[1].rm_so, PMatch[1].rm_eo - PMatch[1].rm_so );
                std::string sValue( _sVal.c_str() + PMatch[2].rm_so, PMatch[2].rm_eo - PMatch[2].rm_so );
                // We want to be case insensitive
                to_lower( sKey );

                trim<std::string>( &sValue, '\t' );
                trim<std::string>( &sValue, ' ' );
                // insert key-value if found
                m_values.insert( keyvalue_t::value_type( sKey, sValue ) );
                return true;
            }

        private:
            ifstream_ptr m_f;
            regex_t m_re;
            keyvalue_t m_values;
    };
    /**
     *
     *
     */
    struct SFindName: public std::binary_function< CProcList::ProcContainer_t::value_type, std::string, bool >
    {
        bool operator()( CProcList::ProcContainer_t::value_type _pid, const std::string &_Name ) const
        {
            CProcStatus p;
            p.Open( _pid );
            return ( p.GetValue( "Name" ) == _Name );
        }
    };
    /**
     *
     *
     */
    typedef std::vector<pid_t> vectorPid_t;

    inline vectorPid_t getprocbyname( const std::string &_Srv )
    {
        CProcList::ProcContainer_t pids;
        CProcList::GetProcList( &pids );

        vectorPid_t retVal;
        CProcList::ProcContainer_t::const_iterator iter = pids.begin();
        while( true )
        {
            iter = std::find_if( iter, pids.end(), std::bind2nd( SFindName(), _Srv ) );
            if( pids.end() == iter )
                break;

            retVal.push_back( *iter );
            ++iter;
        };

        return retVal;
    }

    inline bool is_status_ok( int status )
    {
        return WIFEXITED( status ) && WEXITSTATUS( status ) == 0;
    }

    //TODO: Document me!
    inline void do_execv( const std::string &_Command, const StringVector_t &_Params, size_t _Delay, std::string *_output ) throw( std::exception )
    {
        pid_t child_pid;
        std::vector<const char*> cargs; //careful with c_str()!!!
        cargs.push_back( _Command.c_str() );
        StringVector_t::const_iterator iter = _Params.begin();
        StringVector_t::const_iterator iter_end = _Params.end();
        for( ; iter != iter_end; ++iter )
            cargs.push_back( iter->c_str() );
        cargs.push_back( 0 );

        int fdpipe[2];
        if( _output )
            pipe( fdpipe );

        switch( child_pid = fork() )
        {
            case - 1:
                close( fdpipe[0] );
                close( fdpipe[1] );
                // Unable to fork
                throw std::runtime_error( "do_execv: Unable to fork process" );

            case 0:
                if( _output )
                {
                    close( fdpipe[0] );
                    dup2( fdpipe[1], STDOUT_FILENO );
                    close( fdpipe[1] );
                }

                // child: execute the required command, on success does not return
                execv( _Command.c_str(), const_cast<char **>( &cargs[0] ) );
                ::exit( 1 );
        }

        //parent
        if( _output )
        {
            close( fdpipe[1] );
            char buf;
            std::stringstream ss;
            while( read( fdpipe[0], &buf, 1 ) > 0 )
                ss << buf;

            *_output = ss.str();

        }
        for( size_t i = 0; i < _Delay; ++i )
        {
            int stat;
            if( child_pid == ::waitpid( child_pid, &stat, WNOHANG ) )
            {
                if( !is_status_ok( stat ) )
                {
                    std::stringstream ss;
                    ss << "do_execv: Can't execute \"" << _Command << "\" with parameters: ";
                    std::copy( _Params.begin(), _Params.end(), std::ostream_iterator<std::string>( ss, " " ) );
                    throw std::runtime_error( ss.str() );
                }
                return;
            }
            //TODO: Needs to be fixed! Implement time-function based timeout measurements instead
            sleep( 1 );
        }
        throw std::runtime_error( "do_execv: Timeout has been reached, command execution will be terminated now." );
        //kills the child
        kill( child_pid, SIGKILL );
    }

};

#endif /*PROCESS_H_*/
