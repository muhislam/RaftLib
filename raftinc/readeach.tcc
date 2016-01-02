/**
 * read_each.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct 26 15:51:46 2014
 * 
 * Copyright 2014 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _READEACH_TCC_
#define _READEACH_TCC_  1
#include <iterator>
#include <functional>
#include <map>
#include <cstddef>

#include <list>
#include <vector>
#include <array>
#include <deque>
#include <forward_list>

#include <typeinfo>

#include <raft>
#include "split.tcc"

namespace raft{

template < class T > class read_each : public raft::parallel_k 
{

using it_list    = typename std::list< T >::iterator        ;   
using it_vect    = typename std::vector< T >::iterator      ;   
using it_deq     = typename std::deque< T >::iterator       ;   

template < class iterator_type >
static bool  inc_helper( iterator_type &begin, iterator_type &end, Port &port_list )
{
   for( auto &port : port_list )
   {
      if( begin == end )
      {
         return( true );
      }
      port.push< T >( (*begin) );
      ++begin;
   }
   return( false );
}

/** k, we're going to have to code up mult. structs for each type above **/
template < class X > struct it_cont
{
   it_cont( X * const b, X * const e ): begin( *b ), end( *e ){}
   X begin;
   X end;
};
using it_list_cont    = it_cont< it_list >;   
using it_vect_cont    = it_cont< it_vect >;   
using it_deq_cont     = it_cont< it_deq >;   

const std::map< std::size_t,
         std::function< void* ( void*, void* ) > > cont_map
            =  {
                  { 
                     typeid( it_list ).hash_code(),
                     [ ]( void *b, void *e )
                     {
                        auto *begin = reinterpret_cast< it_list* >( b );
                        auto *end   = reinterpret_cast< it_list* >( e );
                        return( new it_list_cont( begin, end ) );
                     }
                  },
                  { 
                     typeid( it_vect ).hash_code(),
                     [ ]( void *b, void *e )
                     {
                        auto *begin = reinterpret_cast< it_vect* >( b );
                        auto *end   = reinterpret_cast< it_vect* >( e );
                        return( new it_vect_cont( begin, end ) );
                     }
                  },
                  { 
                     typeid( it_deq ).hash_code(),
                     [ ]( void *b, void *e )
                     {
                        auto *begin = reinterpret_cast< it_deq* >( b );
                        auto *end   = reinterpret_cast< it_deq* >( e );
                        return( new it_deq_cont( begin, end ) );
                     }
                  }
               };

const std::map< std::size_t,
         std::function< bool ( void*, Port& ) > > func_map
            =  {
                  { 
                     typeid( it_list ).hash_code(),
                     [ ]( void * const global_cont, Port &port_list )
                     {
                        auto *ptr = 
                           reinterpret_cast< it_list_cont* >( global_cont );
                        return( inc_helper( ptr->begin, 
                                            ptr->end, 
                                            port_list ) );
                     }
                  },
                  { 
                     typeid( it_vect ).hash_code(),
                     [ ]( void * const global_cont, Port &port_list )
                     {
                        auto *ptr = 
                           reinterpret_cast< it_vect_cont* >( global_cont );
                        return( inc_helper( ptr->begin, 
                                            ptr->end, 
                                            port_list ) );
                     }
                  },
                  { 
                     typeid( it_deq ).hash_code(),
                     [ ]( void * const global_cont, Port &port_list )
                     {
                        auto *ptr = 
                           reinterpret_cast< it_deq_cont* >( global_cont );
                        return( inc_helper( ptr->begin, 
                                            ptr->end, 
                                            port_list ) );
                     }
                  }
               };

public:
   template < class iterator_type >
   read_each( iterator_type &&begin, iterator_type &&end ) :
      parallel_k()
   {     
      /** NOTE, single output port is added by split< T > sub-class **/
      addPortTo< T >( output );
      /** 
       * hacky way of getting the right iterator type for the ptr
       * pehaps change if I can figure out how to do without having
       * to move the constructor template to the class template 
       * param
       */
       const auto hash_code( typeid( iterator_type ).hash_code() );
       const auto cont_ret_val( cont_map.find( hash_code ) );
       if( cont_ret_val != cont_map.end() )
       {
          auto cont_getter_func = (*cont_ret_val).second;
          /** set objects **/
          container_ptr = cont_getter_func( &begin, &end );
       }
       else
       {
          assert( false );
       }
       const auto ret_val( func_map.find( hash_code ) );
       if( ret_val != func_map.end() )
       {
          inc_func = (*ret_val).second; 
       }
       else
       {
          /** TODO, make exception for this **/
          assert( false );
       }
   }

   virtual raft::kstatus run()
   {
      if( inc_func( container_ptr, output ) )
      {
         return( raft::stop );
      }
      return( raft::proceed );
   }

protected:
   virtual std::size_t  addPort()
   {
      return( (this)->template addPortTo< T >( (this)->output ) );
   }

private:
   void * container_ptr = nullptr;
   std::function< bool ( void*, Port& ) > inc_func;

};
} /** end namespace raft **/
#endif /* END _READEACH_TCC_ */
