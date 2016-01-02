#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <raft>
#include <raftio>

int
main( int argc, char **argv )
{
   using namespace raft;
   using type_t = std::uint32_t;

   std::vector< type_t > v;
   int i( 0 );
   auto func( [&](){ return( i++ ); } );
   while( i < 10){ v.push_back( func() ); }
   std::vector< type_t > o;
   /** link iterator reader to print kernel **/
   auto b( v.begin() );
   auto e( v.end()   );
   raft::read_each< type_t >  re( b, e );
   raft::write_each< type_t > we( std::back_inserter( o ) );
   raft::map m;
   m += re >> we;
   m.exe();
   
   /** once function returns, o should be readable **/
   for( auto val : o )
   {
      std::cout << val << "\n";
   }
   return( EXIT_SUCCESS );
}
