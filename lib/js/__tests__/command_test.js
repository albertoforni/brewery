'use strict';

var Jest  = require("bs-jest/lib/js/src/jest.js");
var Index = require("../src/index.js");

describe("brewery help", (function () {
        Jest.test("it returns help", (function () {
                var res = [""];
                var system_000 = function (s) {
                  res[0] = s;
                  return /* () */0;
                };
                var system_001 = function (_, _$1) {
                  return /* () */0;
                };
                var system = /* record */[
                  system_000,
                  system_001
                ];
                Index.run(system, /* array */[
                      "node",
                      "program",
                      "help"
                    ]);
                return Jest.Expect[/* toBe */2]("here some help")(Jest.Expect[/* expect */0](res[0]));
              }));
        describe("brewery init", (function () {
                return Jest.test("it creates a .brewery.json", (function () {
                              var res = [""];
                              var writeFileRes = [/* tuple */[
                                  "",
                                  ""
                                ]];
                              var system_000 = function (s) {
                                res[0] = s;
                                return /* () */0;
                              };
                              var system_001 = function (path, content) {
                                writeFileRes[0] = /* tuple */[
                                  path,
                                  content
                                ];
                                return /* () */0;
                              };
                              var system = /* record */[
                                system_000,
                                system_001
                              ];
                              Index.run(system, /* array */[
                                    "node",
                                    "program",
                                    "init"
                                  ]);
                              return Jest.Expect[/* toEqual */12](/* tuple */[
                                            /* tuple */[
                                              Index.breweryConfig,
                                              "{cask: [], brew: []}"
                                            ],
                                            ".brewery.json created"
                                          ])(Jest.Expect[/* expect */0](/* tuple */[
                                              writeFileRes[0],
                                              res[0]
                                            ]));
                            }));
              }));
        return /* () */0;
      }));

/*  Not a pure module */
