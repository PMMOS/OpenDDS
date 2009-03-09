/*
 * $Id$
 */

#include "erl_utility.h"
#include "generator_erl.h"

using namespace std;

generator_erl::generator_erl()
{
}

generator_erl::~generator_erl()
{
}

bool
generator_erl::generate_module(AST_Module* node, vector<AST_Constant*>& v)
{
  // Generate header (.hrl)
  erl_header header(node->name());

  ostream& os = header.open_stream();
  if (!os) return false; // bad stream

  /// Generate constants
  for (vector<AST_Constant*>::iterator it(v.begin()); it != v.end(); ++it)
  {
    os << "-define(" << (*it)->local_name()->get_string() << ", "
       << erl_literal((*it)->constant_value()) << ")." << endl;
  }

  return true;
}

bool
generator_erl::generate_enum(AST_Enum* node, vector<AST_EnumVal*>& v)
{
  // Generate module (.erl)
  erl_module module(node->name());

  /// AST_EnumVal exports (arity always 0)
  module.add_exports(v.begin(), v.end(), 0);
  
  module.add_export("from_int/1");
  module.add_export("value/1");

  ostream& os = module.open_stream();
  if (!os) return false; // bad stream

  for (vector<AST_EnumVal*>::iterator it(v.begin()); it != v.end(); ++it)
  {
    os << erl_identifier((*it)->local_name()) << "() -> {?MODULE, " <<
          erl_literal((*it)->constant_value()) << "}." << endl;
  }

  os << endl
     << "from_int(I) -> {?MODULE, I}." << endl
     << endl
     << "value({?MODULE, I}) -> I." << endl;

  return true;
}

bool
generator_erl::generate_structure(AST_Structure* node, vector<AST_Field*>& v)
{
  erl_header header(node->name());
  erl_module module(node->name());

  erl_identifier_list fields(v.begin(), v.end());
 
  { // Generate header (.hrl)
    ostream& os = header.open_stream();
    if (!os) return false; // bad stream

    os << "-record(" << module << ", {" << fields << "})." << endl;
  }

  { // Generate module (.erl)
    module.add_include(header.basename());
    
    module.add_export("id/0");
    module.add_export("new/0");
    module.add_export("new", fields.size());

    ostream& os = module.open_stream();
    if (!os) return false; // bad stream

    os << "id() -> \"" << node->repoID() << "\"." << endl
       << endl;

    os << "new() -> #" << module << "{}." << endl
       << endl;

    os << "new(" << fields.as_param_list() << ") -> #" << module <<
          "{" << fields.as_init_list() << "}." << endl;
  }

  return true;
}
