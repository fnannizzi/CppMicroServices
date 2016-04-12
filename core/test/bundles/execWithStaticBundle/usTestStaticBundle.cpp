/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include "usTestBundleService.h"

#include <usBundleActivator.h>
#include <usBundleContext.h>
#include <usLog.h>

namespace us {

struct TestBundleImportedByExec : public TestBundleService
{

  TestBundleImportedByExec() {}
  virtual ~TestBundleImportedByExec() {}

};

class TestBundleImportedByExecActivator : public BundleActivator
{
public:

  TestBundleImportedByExecActivator() {}
  ~TestBundleImportedByExecActivator() {}

  void Start(BundleContext* context)
  {
    s = std::make_shared<TestBundleImportedByExec>();
    US_INFO << "Registering TestBundleImportedByExec";
    sr = context->RegisterService<TestBundleImportedByExec>(s);
  }

  void Stop(BundleContext*)
  {
    sr.Unregister();
  }

private:
  std::shared_ptr<TestBundleImportedByExec> s;
  ServiceRegistration<TestBundleImportedByExec> sr;
};

}

US_EXPORT_BUNDLE_ACTIVATOR(us::TestBundleImportedByExecActivator)
