/*
 * Copyright (c) 2015, Cryptonomex, Inc.
 * All rights reserved.
 *
 * This source code is provided for evaluation in private test networks only, until September 8, 2015. After this date, this license expires and
 * the code may not be used, modified or distributed for any purpose. Redistribution and use in source and binary forms, with or without modification,
 * are permitted until September 8, 2015, provided that the following conditions are met:
 *
 * 1. The code and/or derivative works are used only for private test networks consisting of no more than 10 P2P nodes.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <graphene/chain/transaction_object.hpp>

namespace graphene { namespace chain {

const object* transaction_index::create(const std::function<void (object*)>& constructor, object_id_type)
{
   transaction_object obj;

   obj.id = get_next_available_id();
   constructor(&obj);

   auto result = _index.insert(std::move(obj));
   FC_ASSERT(result.second, "Could not create transaction_object! Most likely a uniqueness constraint is violated.");
   return &*result.first;
}

void transaction_index::modify(const object* obj,
                               const std::function<void (object*)>& m)
{
   assert(obj != nullptr);
   FC_ASSERT(obj->id < _index.size());

   const transaction_object* t = dynamic_cast<const transaction_object*>(obj);
   assert(t != nullptr);

   auto itr = _index.find(obj->id.instance());
   assert(itr != _index.end());
   _index.modify(itr, [&m](transaction_object& o) { m(&o); });
}

void transaction_index::add(unique_ptr<object> o)
{
   assert(o);
   object_id_type id = o->id;
   assert(id.space() == transaction_object::space_id);
   assert(id.type() == transaction_object::type_id);
   assert(id.instance() == size());

   auto trx = dynamic_cast<transaction_object*>(o.get());
   assert(trx != nullptr);
   o.release();

   auto result = _index.insert(std::move(*trx));
   FC_ASSERT(result.second, "Could not insert transaction_object! Most likely a uniqueness constraint is violated.");
}

void transaction_index::remove(object_id_type id)
{
   auto& index = _index.get<instance>();
   auto itr = index.find(id.instance());
   if( itr == index.end() )
      return;

   assert(id.space() == transaction_object::space_id);
   assert(id.type() == transaction_object::type_id);

   index.erase(itr);
}

const object*transaction_index::get(object_id_type id) const
{
   if( id.type() != transaction_object::type_id ||
       id.space() != transaction_object::space_id )
      return nullptr;

   auto itr = _index.find(id.instance());
   if( itr == _index.end() )
      return nullptr;
   return &*itr;
}

} } // graphene::chain
