open Jest;

open Index;

open Expect;

let defaultSystem = {
  log: (_) => (),
  writeFile: (_, _) => (),
  exec: (_) => "",
  fileExists: (_) => false,
  readFile: (_) => ""
};

let _ =
  describe("brewery help", () => {
    let expectedFirstLineOutput = {js|Hi from brewery 🍻  here some help|js};
    test("it returns help", () => {
      let logs = ref("");
      let system = {
        ...defaultSystem,
        log: s => {
          logs := s;
          ();
        }
      };
      Index.run(system, [|"node", "program", "help"|]);
      expect(logs^) |> toContainString(expectedFirstLineOutput);
    });
    test("it returns help when no argument is passed", () => {
      let logs = ref("");
      let system = {
        ...defaultSystem,
        log: s => {
          logs := s;
          ();
        }
      };
      Index.run(system, [|"node", "program"|]);
      expect(logs^) |> toContainString(expectedFirstLineOutput);
    });
  });

describe("brewery init", () => {
  test("installs brew when it's not there", () => {
    let installBrew = ref(false);
    let installCask = ref(false);
    let system = {
      ...defaultSystem,
      exec: command =>
        switch command {
        | "brew --version" => assert false /* when it errors out it means it is not installed */
        | command when command == Index.installBrewScript =>
          installBrew := true;
          "install script output";
        | command when command == Index.installCaskScript =>
          installCask := true;
          "install script output";
        | "brew leaves" => "first\nsecond\n"
        | "brew cask list" => "3\n4\n"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "init"|]);
    expect((installBrew^, installCask^)) |> toEqual((true, true));
  });
  test("creates a .breweryfile.json with the returned brew formulas", () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      writeFile: (path, content) => writeFileRes := (path, content),
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | "brew leaves" => "first\nsecond\n"
        | "brew cask list" => "3\n4\n"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "init"|]);
    let breweryfile =
      switch (
        Utils.jsonStringfy({
          "cask": [|"3", "4"|],
          "brew": [|"first", "second"|]
        })
      ) {
      | Some(s) => s
      | None => ""
      };
    expect((writeFileRes^, logs^))
    |> toEqual((
         (Index.breweryfilePath, breweryfile),
         ".breweryfile.json created"
       ));
  });
  test(
    "returns an error when brew leaves throws and does not write .breweryfile",
    () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      writeFile: (path, content) => writeFileRes := (path, content),
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | "brew leaves" => assert false
        | "brew cask list" => "3\n4\n"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "init"|]);
    expect((writeFileRes^, logs^))
    |> toEqual((("", ""), "error getting installed formulas"));
  });
  test(
    "returns an error when brew cask install throws and does not write .breweryfile",
    () => {
      let logs = ref("");
      let writeFileRes = ref(("", ""));
      let system = {
        ...defaultSystem,
        log: s => {
          logs := s;
          ();
        },
        writeFile: (path, content) => writeFileRes := (path, content),
        exec: command =>
          switch command {
          | "brew --version" => "brew already installed"
          | "brew leaves" => "3\n4\n"
          | "brew cask list" => assert false
          | _ => ""
          }
      };
      Index.run(system, [|"node", "program", "init"|]);
      expect((writeFileRes^, logs^))
      |> toEqual((("", ""), "error getting installed formulas"));
    }
  );
  test("returns an error if .breweryfile.json is already there", () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      writeFile: (path, content) => writeFileRes := (path, content),
      fileExists: path =>
        if (path == Index.breweryfilePath) {
          true;
        } else {
          assert false;
        }
    };
    Index.run(system, [|"node", "program", "init"|]);
    expect((writeFileRes^, logs^))
    |> toEqual((("", ""), Index.breweryfilePath ++ " exists already"));
  });
});

describe("brewery install", () => {
  let initialBrewery =
    Utils.jsonStringfy({"cask": [|"3"|], "brew": [|"first", "second"|]})
    |> (
      res =>
        switch res {
        | Some(content) => content
        | None => "error"
        }
    );
  test("installs brew when it's not there", () => {
    let installBrew = ref(false);
    let installCask = ref(false);
    let system = {
      ...defaultSystem,
      exec: command =>
        switch command {
        | "brew --version" => assert false /* when it errors it means it is not installed */
        | command when command == Index.installBrewScript =>
          installBrew := true;
          "install script output";
        | command when command == Index.installCaskScript =>
          installCask := true;
          "install script output";
        | "brew leaves" => "first\nsecond\n"
        | "brew cask list" => "3\n4\n"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "install"|]);
    expect((installBrew^, installCask^)) |> toEqual((true, true));
  });
  test("adds brew formula to .breweryfile.json", () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let readFileRes = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      writeFile: (path, content) => writeFileRes := (path, content),
      readFile: path => {
        readFileRes := path;
        initialBrewery;
      },
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "install", "yarn"|]);
    let breweryfile =
      switch (
        Utils.jsonStringfy({
          "cask": [|"3"|],
          "brew": [|"first", "second", "yarn"|]
        })
      ) {
      | Some(s) => s
      | None => ""
      };
    expect((writeFileRes^, logs^))
    |> toEqual((
         (Index.breweryfilePath, breweryfile),
         ".breweryfile.json updated"
       ));
  });
  test("adds cask formula to .breweryfile.json", () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let readFileRes = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      writeFile: (path, content) => writeFileRes := (path, content),
      readFile: path => {
        readFileRes := path;
        initialBrewery;
      },
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "install", "cask", "yay"|]);
    let breweryfile =
      switch (
        Utils.jsonStringfy({
          "cask": [|"3", "yay"|],
          "brew": [|"first", "second"|]
        })
      ) {
      | Some(s) => s
      | None => ""
      };
    expect((writeFileRes^, logs^))
    |> toEqual((
         (Index.breweryfilePath, breweryfile),
         ".breweryfile.json updated"
       ));
  });
  test(
    "installs all formulas from .breweryfile.json when no arguments are passed in",
    () => {
    let initialBrewery =
      Utils.jsonStringfy({"cask": [|"3", "foo"|], "brew": [|"first", "boom"|]})
      |> (
        res =>
          switch res {
          | Some(content) => content
          | None => "error"
          }
      );
    let logs = ref([]);
    let readFileRes = ref("");
    let installedFormulas = ref([]);
    let system = {
      ...defaultSystem,
      log: s => {
        logs := logs^ @ [s];
        ();
      },
      readFile: path => {
        readFileRes := path;
        initialBrewery;
      },
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | formula =>
          installedFormulas := installedFormulas^ @ [formula];
          "";
        }
    };
    Index.run(system, [|"node", "program", "install"|]);
    expect((logs^, installedFormulas^))
    |> toEqual((
         [
           "first installed successfully",
           "boom installed successfully",
           "3 installed successfully",
           "foo installed successfully",
           "All formulas installed"
         ],
         [
           "brew install first",
           "brew install boom",
           "brew cask install 3",
           "brew cask install foo"
         ]
       ));
  });
  test("logs the formulas that failed to be installed", () => {
    let initialBrewery =
      Utils.jsonStringfy({"cask": [|"3", "foo"|], "brew": [|"first", "boom"|]})
      |> (
        res =>
          switch res {
          | Some(content) => content
          | None => "error"
          }
      );
    let logs = ref([]);
    let readFileRes = ref("");
    let installedFormulas = ref([]);
    let system = {
      ...defaultSystem,
      log: s => {
        logs := logs^ @ [s];
        ();
      },
      readFile: path => {
        readFileRes := path;
        initialBrewery;
      },
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | formula =>
          if (formula != "brew cask install 3") {
            installedFormulas := installedFormulas^ @ [formula];
            "";
          } else {
            installedFormulas := installedFormulas^ @ [formula];
            raise(Exit);
          }
        }
    };
    Index.run(system, [|"node", "program", "install"|]);
    expect((logs^, installedFormulas^))
    |> toEqual((
         [
           "first installed successfully",
           "boom installed successfully",
           "Error installing 3 formula",
           "foo installed successfully",
           "Not all formulas installed"
         ],
         [
           "brew install first",
           "brew install boom",
           "brew cask install 3",
           "brew cask install foo"
         ]
       ));
  });
  test("doesn't add cask formula to .breweryfile.json when already there", () => {
    let initialBrewery =
      Utils.jsonStringfy({
        "cask": [|"3", "yay"|],
        "brew": [|"first", "second"|]
      })
      |> (
        res =>
          switch res {
          | Some(content) => content
          | None => "error"
          }
      );
    let writeFileRes = ref(("", ""));
    let system = {
      ...defaultSystem,
      writeFile: (path, content) => writeFileRes := (path, content),
      readFile: (_) => initialBrewery,
      exec: command =>
        switch command {
        | "brew --version" => "brew already installed"
        | _ => ""
        }
    };
    Index.run(system, [|"node", "program", "install", "cask", "yay"|]);
    let breweryfile =
      switch (
        Utils.jsonStringfy({
          "cask": [|"3", "yay"|],
          "brew": [|"first", "second"|]
        })
      ) {
      | Some(s) => s
      | None => ""
      };
    expect(writeFileRes^) |> toEqual((Index.breweryfilePath, breweryfile));
  });
  test("returns an error if .breweryfile.json isn't found", () => {
    let logs = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      readFile: (_) => raise(Not_found)
    };
    let breweryfilePath = Index.breweryfilePath;
    Index.run(system, [|"node", "program", "install", "foo"|]);
    expect(logs^) |> toEqual({j|Error loading $breweryfilePath|j});
  });
});

describe("brewery list", () => {
  let breweryConf =
    Utils.jsonStringfy({"cask": [|"3"|], "brew": [|"first", "second"|]})
    |> (
      res =>
        switch res {
        | Some(content) => content
        | None => "error"
        }
    );
  test("shows installed formulas", () => {
    let logs = ref("");
    let readFileRes = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      readFile: path => {
        readFileRes := path;
        breweryConf;
      }
    };
    Index.run(system, [|"node", "program", "list"|]);
    expect((readFileRes^, logs^))
    |> toEqual((Index.breweryfilePath, breweryConf));
  });
  test("returns an error if .breweryfile.json isn't found", () => {
    let logs = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      readFile: (_) => raise(Not_found)
    };
    let breweryfilePath = Index.breweryfilePath;
    Index.run(system, [|"node", "program", "list"|]);
    expect(logs^) |> toEqual({j|Error loading $breweryfilePath|j});
  });
});

describe("brewery uninstall", () => {
  let toJson = brewery =>
    Utils.jsonStringfy(brewery)
    |> (
      res =>
        switch res {
        | Some(content) => content
        | None => "error"
        }
    );
  let initialBrewery =
    {"cask": [|"3", "foo"|], "brew": [|"pluto", "tom"|]} |> toJson;
  test("removes the formula", () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      readFile: path => path == Index.breweryfilePath ? initialBrewery : "",
      writeFile: (path, content) => writeFileRes := (path, content)
    };
    let expectedBrewery =
      {"cask": [|"3", "foo"|], "brew": [|"tom"|]} |> toJson;
    Index.run(system, [|"node", "program", "uninstall", "pluto"|]);
    expect((logs^, writeFileRes^))
    |> toEqual((
         ".breweryfile.json updated",
         (Index.breweryfilePath, expectedBrewery)
       ));
  });
  test("removes the cask formula", () => {
    let logs = ref("");
    let writeFileRes = ref(("", ""));
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      readFile: path => path == Index.breweryfilePath ? initialBrewery : "",
      writeFile: (path, content) => writeFileRes := (path, content)
    };
    let expectedBrewery =
      {"cask": [|"3"|], "brew": [|"pluto", "tom"|]} |> toJson;
    Index.run(system, [|"node", "program", "uninstall", "cask", "foo"|]);
    expect((logs^, writeFileRes^))
    |> toEqual((
         ".breweryfile.json updated",
         (Index.breweryfilePath, expectedBrewery)
       ));
  });
  test("returns an error if .breweryfilePath.json isn't found", () => {
    let logs = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      },
      readFile: (_) => raise(Not_found)
    };
    let breweryConfig = Index.breweryfilePath;
    Index.run(system, [|"node", "program", "uninstall", "pluto"|]);
    expect(logs^) |> toEqual({j|Error loading $breweryConfig|j});
  });
});

describe("unknown command", () =>
  test("returns an error message when a non command is passed in", () => {
    let logs = ref("");
    let system = {
      ...defaultSystem,
      log: s => {
        logs := s;
        ();
      }
    };
    Index.run(system, [|"node", "program", "foo", "pluto"|]);
    expect(logs^) |> toEqual({j|I don't know the foo command|j});
  })
);