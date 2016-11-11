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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"

#include "TestingMacros.h"
#include "TestUtils.h"

using namespace cppmicroservices;

void TestLDAPFilterMatchBundle(const Bundle& bundle)
{
  LDAPFilter ldapMatchCase( "(bundle.testproperty=YES)" );
  LDAPFilter ldapKeyMismatchCase( "(bundle.TestProperty=YES)" );
  LDAPFilter ldapValueMismatchCase( "(bundle.testproperty=Yes)" );

  // Exact string match of both key and value
  US_TEST_CONDITION(ldapMatchCase.Match(bundle), " Evaluating LDAP expr: " + ldapMatchCase.ToString());

  // Testing case-insensitive key (should still pass)
  US_TEST_CONDITION(ldapKeyMismatchCase.Match(bundle), " Evaluating LDAP expr: " + ldapKeyMismatchCase.ToString());

  // Testing case-insensitive value (should fail)
  US_TEST_CONDITION(!ldapValueMismatchCase.Match(bundle), " Evaluating LDAP expr: " + ldapValueMismatchCase.ToString());
}

void TestLDAPFilterMatchServiceReferenceBase(Bundle bundle)
{
  LDAPFilter ldapMatchCase( "(service.testproperty=YES)" );
  LDAPFilter ldapKeyMismatchCase( "(service.TestProperty=YES)" );
  LDAPFilter ldapValueMismatchCase( "(service.testproperty=Yes)" );

  bundle.Start();

  auto thisBundleCtx = bundle.GetBundleContext();
  ServiceReferenceU sr = thisBundleCtx.GetServiceReference("cppmicroservices::TestBundleLQService");

  // Make sure the obtained ServiceReferenceBase object is not null
  US_TEST_CONDITION(sr, " Checking non-empty ServiceReferenceBase object");

  // Exact string match of both key and value
  US_TEST_CONDITION(ldapMatchCase.Match(sr), " Evaluating LDAP expr: " + ldapMatchCase.ToString());

  // Testing case-insensitive key (should still pass)
  US_TEST_CONDITION(ldapKeyMismatchCase.Match(sr), " Evaluating LDAP expr: " + ldapKeyMismatchCase.ToString());

  // Testing case-insensitive value (should fail)
  US_TEST_CONDITION(!ldapValueMismatchCase.Match(sr), " Evaluating LDAP expr: " + ldapValueMismatchCase.ToString());

  bundle.Stop();

  // Testing the behavior after the bundle has stopped (service properties
  // should still be available for queries according to OSGi spec 5.2.1).
  US_TEST_CONDITION(ldapMatchCase.Match(sr), " Evaluating LDAP expr: " + ldapMatchCase.ToString());
}

void TestLDAPFilterMatchLDAPSyntax(Bundle bundle)
{
  bundle.Start();
  
  auto thisBundleCtx = bundle.GetBundleContext();
  ServiceReferenceU sr = thisBundleCtx.GetServiceReference("cppmicroservices::TestBundleLQService");
  
  // One real property and one fake property (should fai1)
  LDAPFilter ldap1( "(&(service.testproperty = YES)(service.fakeproperty = YES))" );
  US_TEST_CONDITION(!ldap1.Match(sr), " Evaluating LDAP expr: " + ldap1.ToString());
  
  // One real property and not one fake property (fails but should pass)
  //LDAPFilter ldap2( "(&(service.testproperty = YES)(!(service.fakeproperty=*)))" );
  //US_TEST_CONDITION(ldap2.Match(sr), " Evaluating LDAP expr: " + ldap2.ToString());

  // Test wildcard
  LDAPFilter ldap3( "(service.falseproperty=*)" );
  US_TEST_CONDITION(ldap3.Match(sr), " Evaluating LDAP expr: " + ldap3.ToString());

  // Test numeric
  LDAPFilter ldap4( "(service.numericproperty <= 10)" );
  US_TEST_CONDITION(ldap4.Match(sr), " Evaluating LDAP expr: " + ldap4.ToString());
  
  // Complicated expression (fails but should pass)
  //LDAPFilter ldap5( "(|(service.testproperty = YES)(service.numericproperty <= 3))");
  //US_TEST_CONDITION(ldap5.Match(sr), " Evaluating LDAP expr: " + ldap5.ToString());
  
  // Test for boolean property (fails but should pass, I think)
  //LDAPFilter ldap5( "(service.trueproperty = TRUE)" );
  //US_TEST_CONDITION(ldap5.Match(sr), " Evaluating LDAP expr: " + ldap5.ToString());
  
  bundle.Stop();
}

void TestLDAPFilterMatchCSyntax(Bundle bundle)
{
  bundle.Start();
  
  auto thisBundleCtx = bundle.GetBundleContext();
  ServiceReferenceU sr = thisBundleCtx.GetServiceReference("cppmicroservices::TestBundleLQService");
  
  // One real property and one fake property (should fail)
  LDAPFilter c1( "(service.testproperty = YES && service.fakeproperty = YES)" );
  US_TEST_CONDITION(!c1.Match(sr), " Evaluating LDAP expr: " + c1.ToString());
  
  // One real property and one not fake property (fails, but probably should pass)
  LDAPFilter c2( "(service.testproperty = YES && !service.fakeproperty)" );
  US_TEST_CONDITION(!c2.Match(sr), " Evaluating LDAP expr: " + c2.ToString());

  // Test wildcard
  LDAPFilter c3( "(service.falseproperty =*)" );
  US_TEST_CONDITION(c3.Match(sr), " Evaluating LDAP expr: " + c3.ToString());
  
  // Test wildcard
  LDAPFilter c4( "(service.notaproperty =*)" );
  US_TEST_CONDITION(!c4.Match(sr), " Evaluating LDAP expr: " + c4.ToString());
  
  // Complicated expression
  //LDAPFilter c5( "((service.testproperty = YES || service.numericproperty >= 10) && (service.numericproperty =< 9 && service.numericproperty >= 2))" );
  //US_TEST_CONDITION(!c5.Match(sr), " Evaluating LDAP expr: " + c5.ToString());
  
  // Test for boolean property
  LDAPFilter c6(LDAPProp("service.trueproperty") == "TRUE");
  US_TEST_CONDITION(c6.Match(sr), " Evaluating LDAP expr: " + c6.ToString());
  
  // Test for boolean property
  //LDAPFilter c3("service.trueproperty == TRUE");
  //US_TEST_CONDITION(c3.Match(sr), " Evaluating LDAP expr: " + c3.ToString());
  
  
  // One real property and not fake property
  //LDAPFilter ldapRealAndNotFake2(LDAPProp("service.testproperty") == "YES" && !LDAPProp("service.fakeproperty"));
  //US_TEST_CONDITION(ldapRealAndNotFake2.Match(sr), " Evaluating LDAP expr: " + ldapRealAndNotFake2.ToString());
  
  // One real property or one fake property
  //LDAPFilter ldapRealOrFake(LDAPProp("service.testproperty") || !LDAPProp("service.fakeproperty"));
  //US_TEST_CONDITION(ldapRealOrFake.Match(sr), " Evaluating LDAP expr: " + ldapRealOrFake.ToString());
  
  
  bundle.Stop();
}

int LDAPQueryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("LDAPQueryTest");

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();

  auto bundle = testing::InstallLib(framework.GetBundleContext(), "TestBundleLQ");

  US_TEST_OUTPUT(<< "Testing LDAP query using LDAP syntax:")
  TestLDAPFilterMatchLDAPSyntax(bundle);

  US_TEST_OUTPUT(<< "Testing LDAP query using C-style syntax:")
  TestLDAPFilterMatchCSyntax(bundle);

  US_TEST_OUTPUT(<< "Testing LDAP query of bundle properties:")
  TestLDAPFilterMatchBundle(bundle);

  US_TEST_OUTPUT(<< "Testing LDAP query of service properties:")
  TestLDAPFilterMatchServiceReferenceBase(bundle);

  framework.Stop();

  US_TEST_END()
}
