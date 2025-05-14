#include "process.hpp"
#include <vector>
#include <cstdint>
 
std::uint32_t
data_processor_t::process_block( const std::vector<std::uint32_t> &block )
{
     std::uint32_t hash = m_last_hash;
 
     for ( std::size_t i = 0; i < block.size(); i++ )
     {
         std::uint32_t part_hash = 0;
         std::uint32_t prev_val = 0;
         std::uint32_t ind = 0;
 
         for ( std::size_t j = i; j < block.size(); j++ )
         {
             if ( 0 == block[j] )
             {
                 break;
             }
 
             if ( prev_val > block[j] )
             {
                 ind = 0;
             }
 
             part_hash ^= block[j] << ind;
             ind++;
 
             prev_val = block[j];
         }
         hash ^= part_hash;
     }
 
     m_last_hash = hash;
     return hash;
}

