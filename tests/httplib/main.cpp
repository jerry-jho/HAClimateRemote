//
//  simplecli.cc
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <httplib.h>
#include <iostream>

void setup() {
  if (auto res = httplib::Client("localhost", 8080).Get("/hi.html")) {
    std::cout << res->status << std::endl;
    std::cout << res->get_header_value("Content-Type") << std::endl;
    std::cout << res->body << std::endl;
  } else {
    std::cout << res.error() << std::endl;
  }
}

void loop() {
  
}