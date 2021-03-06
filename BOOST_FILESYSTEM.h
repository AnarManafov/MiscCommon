/************************************************************************/
/**
 * @file BOOST_FILESYSTEM.h
 * @brief BOOST filesystem library helper
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-08-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
************************************************************************/
#ifndef BOOST_FILESYSTEM_H_
#define BOOST_FILESYSTEM_H_

// BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/exception.hpp"
// MiscCommon
#include "MiscUtils.h"

namespace fs = boost::filesystem;

namespace MiscCommon
{
    namespace BOOSTHelper
    {
        /**
         *
         * @brief The normalize_path() function removes '/' characters at the end of the of the input pathname
         * @param[in] _path - path to process.
         * @return a string, which holds the normalized path.
         *
         */
        inline std::string normalize_path( const std::string &_path )
        {
            std::string path( _path );
            MiscCommon::trim_right<std::string>( &path, '/' );
            return path;
        }
        /**
         *
         * @brief The is_file() function checks whether the pathname represents a file or not.
         * @param[in] _pathname - a full path to check.
         * @return <b>true</b> if the given path represents a file and <b>false</b> otherwise.
         *
         */
        inline bool is_file( const std::string &_pathname )
        {
            bool is_valid = false;

            try
            {
                fs::path cp( normalize_path( _pathname ), fs::native );
                is_valid = !( fs::is_directory( cp ) ) ;
            }
            catch( const fs::filesystem_error &_ex )
                {}

            return is_valid;
        }
        /**
         *
         * @brief the is_directory() function checks whether the pathname represents a directory or not
         * @param[in] _pathname - a path name.
         * @return <b>true</b> if the given path represents a directory and <b>false</b> otherwise.
         *
         */
        inline bool is_directory( const std::string &_pathname )
        {
            bool is_valid = false;
            try
            {
                fs::path cp( normalize_path( _pathname ), fs::native );
                is_valid =  fs::is_directory( cp ) ;
            }
            catch( const fs::filesystem_error &_ex )
                {}

            return is_valid;
        }
    };
};
#endif /*BOOST_FILESYSTEM_H_*/
