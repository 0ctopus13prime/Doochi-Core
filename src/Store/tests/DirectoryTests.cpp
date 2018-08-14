/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gtest/gtest.h>
#include <Store/Directory.h>
#include <iostream>
#include <memory>

using lucene::core::store::MMapDirectory;
using lucene::core::store::IndexInput;
using lucene::core::store::IOContext;

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY) {
  MMapDirectory dir("/root/Doochi-Core/src/Store");

  IOContext io_ctx;
  std::unique_ptr<IndexInput> ptr = dir.OpenInput("DataInput.h", io_ctx);

  std::cout << "Length -> " << ptr->Length() << std::endl;
  for (uint64_t i = 0 ; i < ptr->Length() ; ++i) {
    std::cout << ptr->ReadByte();
  }

  std::cout << "End of printing" << std::endl;
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
