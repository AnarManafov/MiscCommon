/*
 *  logEngine.h
 *  pod-ssh
 *
 *  Created by Anar Manafov on 31.08.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
#ifndef LOGENGINE_H
#define LOGENGINE_H
//=============================================================================
#include <boost/thread/thread.hpp>
#include <csignal>
//=============================================================================
class CLogEngine
{
    public:
        CLogEngine( bool _debugMode = false ):
            m_fd( 0 ),
            m_thread( NULL ),
            m_debugMode( _debugMode ),
            m_stopLogEngine( 0 )
        {}
        ~CLogEngine();
        void start( const std::string &_pipeFilePath );
        void stop();
        void operator()( const std::string &_msg,
                         const std::string &_id = "**",
                         bool _debugMsg = false ) const;
        void debug_msg( const std::string &_msg,
                        const std::string &_id = "**" ) const
        {
            operator()( _msg, _id, true );
        }


    private:
        void thread_worker( int _fd, const std::string & _pipename );

    private:
        int m_fd;
        boost::thread *m_thread;
        std::string m_pipeName;
        bool m_debugMode;
        volatile sig_atomic_t m_stopLogEngine;
};
//=============================================================================
#endif
