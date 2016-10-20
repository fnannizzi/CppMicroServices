/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"

#include "TestingMacros.h"

#include <stdexcept>
#include <chrono>

using namespace cppmicroservices;

int TestParsing()
{
  // WELL FORMED Expr
  try
  {
    US_TEST_OUTPUT(<< "Parsing (cn=Babs Jensen)")
    LDAPFilter ldap( "(cn=Babs Jensen)" );
    US_TEST_OUTPUT(<< "Parsing (!(cn=Tim Howes))")
    ldap = LDAPFilter( "(!(cn=Tim Howes))" );
    US_TEST_OUTPUT(<< "Parsing " << std::string("(&(") + Constants::OBJECTCLASS + "=Person)(|(sn=Jensen)(cn=Babs J*)))")
    ldap = LDAPFilter( std::string("(&(") + Constants::OBJECTCLASS + "=Person)(|(sn=Jensen)(cn=Babs J*)))" );
    US_TEST_OUTPUT(<< "Parsing (o=univ*of*mich*)")
    ldap = LDAPFilter( "(o=univ*of*mich*)" );
  }
  catch (const std::invalid_argument& e)
  {
    US_TEST_OUTPUT(<< e.what());
    return EXIT_FAILURE;
  }


  // MALFORMED Expr
  try
  {
    US_TEST_OUTPUT( << "Parsing malformed expr: cn=Babs Jensen)")
    LDAPFilter ldap( "cn=Babs Jensen)" );
    return EXIT_FAILURE;
  }
  catch (const std::invalid_argument&)
  {
  }

  return EXIT_SUCCESS;
}

bool doEvaluationTest(const char* description, const LDAPFilter &ldap, const ServiceProperties &props) {
    US_TEST_OUTPUT(<< description << ldap.ToString())
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    bool eval = ldap.Match(props);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    US_TEST_OUTPUT(<< " Query took " << std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1 ).count() << " nanoseconds.")
    return eval;
}

int TestEvaluate()
{
  // EVALUATE
  try
  {
    // Make sure Match's key look-up is case-insensitive
    LDAPFilter ldap( "(Cn=Babs Jensen)" );
    ServiceProperties props;

    // Several values
    props["cn"] = std::string("Babs Jensen");
    props["unused"] = std::string("Jansen");
    if (!doEvaluationTest("First query is always slowest. Evaluating expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }
    if (!doEvaluationTest("Repeat of previous query. Evaluating expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // Several more values
    props["a_unused"] = std::string("Bubs Jensen");
    props["b_unused"] = std::string("Jansenn");
    props["c_unused"] = std::string("Bubsy");
    props["d_unused"] = std::string("JanBubs");
    if (!doEvaluationTest("Evaluating extra entries expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // WILDCARD
    ldap = LDAPFilter( "(cn=Babs *)" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    if (!doEvaluationTest("Evaluating wildcard expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // AND
    ldap = LDAPFilter( "(&(cn=Babs *)(dn=Broc*))" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    props["dn"] = std::string("Brock Samson");
    if (!doEvaluationTest("Evaluating and expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // OR
    ldap = LDAPFilter( "(|(cn=Babs *)(dn=Broc*))" );
    props.clear();
    props["cn"] = std::string("Bubs Bransen");
    props["dn"] = std::string("Brock Samson");
    if (!doEvaluationTest("Evaluating or expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // NOT
    ldap = LDAPFilter( "(!(cn=Broc*))" );
    props.clear();
    props["cn"] = std::string("Brock Brabsen");
    props["cn"] = std::string("Brock Samson");
    if (doEvaluationTest("Evaluating not expr: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // NOT FOUND
    ldap = LDAPFilter( "(cn=Babs *)" );
    props.clear();
    props["unused"] = std::string("New");
    if (doEvaluationTest("Expr not found test: ", ldap, props))
    {
        return EXIT_FAILURE;
    }

    // std::vector with integer values
    ldap = LDAPFilter( "  ( |(cn=Babs *)(sn=1) )" );
    props.clear();
    std::vector<Any> list;
    list.push_back(std::string("Babs Jensen"));
    list.push_back(std::string("1"));
    props["sn"] = list;
    if (!doEvaluationTest("Evaluating vector expr: ", ldap, props))
    { 
        return EXIT_FAILURE;
    }

    // wrong case
    ldap = LDAPFilter( "(cN=Babs *)" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    if (!doEvaluationTest("Evaluating case sensitive expr: ", ldap, props))
    { 
        return EXIT_FAILURE;
    }
  }
  catch (const std::invalid_argument& e)
  {
    US_TEST_OUTPUT( << e.what() )
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void TestLDAPExpressions()
{
  LDAPFilter filter(
        LDAPProp("bla") == "jo" && !(LDAPProp("ha") == 1) &&
        (LDAPProp("presence") || !LDAPProp("absence")) &&
        LDAPProp("le") <= 4.1 && LDAPProp("ge") >= -3 &&
        LDAPProp("approx").Approx("Approx")
        );
  const std::string filterStr = "(&(&(&(&(&(bla=jo)(!(ha=1)))(|(presence=*)(!(absence=*))))(le<=4.1))(ge>=-3))(approx~=Approx))";
  US_TEST_CONDITION(filter.ToString() == filterStr, "test generated filter string")

  std::string emptyValue;
  std::string someValue = "some";
  std::string filter1 = LDAPProp("key2") == someValue && LDAPProp("key3");
  std::string filter2 = LDAPProp("key2") == someValue && (LDAPProp("key1") == emptyValue || LDAPProp("key3"));
  US_TEST_CONDITION(filter1 == filter2, "test null expressions")
}

int LDAPFilterTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("LDAPFilterTest");

  TestLDAPExpressions();
  US_TEST_CONDITION(TestParsing() == EXIT_SUCCESS, "Parsing LDAP expressions: ")
  US_TEST_CONDITION(TestEvaluate() == EXIT_SUCCESS, "Evaluating LDAP expressions: ")

  US_TEST_END()
}
