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
  describe(
    "brewery help",
    () =>
      test(
        "it returns help",
        () => {
          let logs = ref("");
          let system = {
            ...defaultSystem,
            log: (s) => {
              logs := s;
              ()
            }
          };
          Index.run(system, [|"node", "program", "help"|]);
          expect(logs^) |> toContainString({js|Hi from brewery 🍻  here some help|js})
        }
      )
  );

describe(
  "brewery init",
  () => {
    test(
      "installs brew when it's not there",
      () => {
        let installBrew = ref(false);
        let system = {
          ...defaultSystem,
          exec: (command) =>
            switch command {
            | "brew --version" => assert false /* when it errors out it means it is not installed */
            | s when s == Index.installBrewScript =>
              installBrew := true;
              "install script output"
            | "brew leaves" => "first\nsecond\n"
            | "brew cask list" => "3\n4\n"
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect(installBrew^) |> toEqual(true)
      }
    );
    test(
      "creates a .brewery.json with the returned brew packages",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          exec: (command) =>
            switch command {
            | "brew --version" => "brew already installed"
            | "brew leaves" => "first\nsecond\n"
            | "brew cask list" => "3\n4\n"
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "init"|]);
        let brewConfigJson =
          switch (Utils.jsonStringfy({"cask": [|"3", "4"|], "brew": [|"first", "second"|]})) {
          | Some(s) => s
          | None => ""
          };
        expect((writeFileRes^, logs^))
        |> toEqual(((Index.breweryConfig, brewConfigJson), ".brewery.json created"))
      }
    );
    test(
      "returns an error when brew leaves throws and does not write .brewconfig",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          exec: (command) =>
            switch command {
            | "brew --version" => "brew already installed"
            | "brew leaves" => assert false
            | "brew cask list" => "3\n4\n"
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect((writeFileRes^, logs^)) |> toEqual((("", ""), "error getting installed formulas"))
      }
    );
    test(
      "returns an error when brew cask install throws and does not write .brewconfig",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          exec: (command) =>
            switch command {
            | "brew --version" => "brew already installed"
            | "brew leaves" => "3\n4\n"
            | "brew cask list" => assert false
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect((writeFileRes^, logs^)) |> toEqual((("", ""), "error getting installed formulas"))
      }
    );
    test(
      "returns an error if .brewery.json is already there",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          fileExists: (path) =>
            if (path == Index.breweryConfig) {
              true
            } else {
              assert false
            }
        };
        Index.run(system, [|"node", "program", "init"|]);
        expect((writeFileRes^, logs^))
        |> toEqual((("", ""), Index.breweryConfig ++ " exists already"))
      }
    )
  }
);

describe(
  "brewery install",
  () => {
    let initialBrewery =
      Utils.jsonStringfy({"cask": [|"3"|], "brew": [|"first", "second"|]})
      |> (
        (res) =>
          switch res {
          | Some(content) => content
          | None => "error"
          }
      );
    test(
      "installs brew when it's not there",
      () => {
        let installBrew = ref(false);
        let system = {
          ...defaultSystem,
          exec: (command) =>
            switch command {
            | "brew --version" => assert false /* when it errors it means it is not installed */
            | s when s == Index.installBrewScript =>
              installBrew := true;
              "install script output"
            | "brew leaves" => "first\nsecond\n"
            | "brew cask list" => "3\n4\n"
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "install"|]);
        expect(installBrew^) |> toEqual(true)
      }
    );
    test(
      "adds brew package to .brewery.json",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let readFileRes = ref("");
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          readFile: (path) => {
            readFileRes := path;
            initialBrewery
          },
          exec: (command) =>
            switch command {
            | "brew --version" => "brew already installed"
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "install", "yarn"|]);
        let brewConfigJson =
          switch (Utils.jsonStringfy({"cask": [|"3"|], "brew": [|"first", "second", "yarn"|]})) {
          | Some(s) => s
          | None => ""
          };
        expect((writeFileRes^, logs^))
        |> toEqual(((Index.breweryConfig, brewConfigJson), ".brewery.json updated"))
      }
    );
    test(
      "adds cask package to .brewery.json",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let readFileRes = ref("");
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          writeFile: (path, content) => writeFileRes := (path, content),
          readFile: (path) => {
            readFileRes := path;
            initialBrewery
          },
          exec: (command) =>
            switch command {
            | "brew --version" => "brew already installed"
            | _ => ""
            }
        };
        Index.run(system, [|"node", "program", "install", "cask", "yay"|]);
        let brewConfigJson =
          switch (Utils.jsonStringfy({"cask": [|"3", "yay"|], "brew": [|"first", "second"|]})) {
          | Some(s) => s
          | None => ""
          };
        expect((writeFileRes^, logs^))
        |> toEqual(((Index.breweryConfig, brewConfigJson), ".brewery.json updated"))
      }
    );
    test(
      "returns an error if .brewery.json isn't found",
      () => {
        let logs = ref("");
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          readFile: (_) => raise(Not_found)
        };
        let breweryConfig = Index.breweryConfig;
        Index.run(system, [|"node", "program", "install", "foo"|]);
        expect(logs^) |> toEqual({j|Error loading $breweryConfig|j})
      }
    )
  }
);

describe(
  "brewery list",
  () => {
    let breweryConf =
      Utils.jsonStringfy({"cask": [|"3"|], "brew": [|"first", "second"|]})
      |> (
        (res) =>
          switch res {
          | Some(content) => content
          | None => "error"
          }
      );
    test(
      "shows installed packages",
      () => {
        let logs = ref("");
        let readFileRes = ref("");
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          readFile: (path) => {
            readFileRes := path;
            breweryConf
          }
        };
        Index.run(system, [|"node", "program", "list"|]);
        expect((readFileRes^, logs^)) |> toEqual((Index.breweryConfig, breweryConf))
      }
    );
    test(
      "returns an error if .brewery.json isn't found",
      () => {
        let logs = ref("");
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          readFile: (_) => raise(Not_found)
        };
        let breweryConfig = Index.breweryConfig;
        Index.run(system, [|"node", "program", "list"|]);
        expect(logs^) |> toEqual({j|Error loading $breweryConfig|j})
      }
    )
  }
);

describe(
  "brewery uninstall",
  () => {
    let toJson = (brewery) =>
      Utils.jsonStringfy(brewery)
      |> (
        (res) =>
          switch res {
          | Some(content) => content
          | None => "error"
          }
      );
    let initialBrewery = {"cask": [|"3", "foo"|], "brew": [|"pluto", "tom"|]} |> toJson;
    test(
      "removes the formula",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          readFile: (path) => path == Index.breweryConfig ? initialBrewery : "",
          writeFile: (path, content) => writeFileRes := (path, content)
        };
        let expectedBrewery = {"cask": [|"3", "foo"|], "brew": [|"tom"|]} |> toJson;
        Index.run(system, [|"node", "program", "uninstall", "pluto"|]);
        expect((logs^, writeFileRes^))
        |> toEqual((".brewery.json updated", (Index.breweryConfig, expectedBrewery)))
      }
    );
    test(
      "removes the cask formula",
      () => {
        let logs = ref("");
        let writeFileRes = ref(("", ""));
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          readFile: (path) => path == Index.breweryConfig ? initialBrewery : "",
          writeFile: (path, content) => writeFileRes := (path, content)
        };
        let expectedBrewery = {"cask": [|"3"|], "brew": [|"pluto", "tom"|]} |> toJson;
        Index.run(system, [|"node", "program", "uninstall", "cask", "foo"|]);
        expect((logs^, writeFileRes^))
        |> toEqual((".brewery.json updated", (Index.breweryConfig, expectedBrewery)))
      }
    );
    test(
      "returns an error if .brewery.json isn't found",
      () => {
        let logs = ref("");
        let system = {
          ...defaultSystem,
          log: (s) => {
            logs := s;
            ()
          },
          readFile: (_) => raise(Not_found)
        };
        let breweryConfig = Index.breweryConfig;
        Index.run(system, [|"node", "program", "uninstall", "pluto"|]);
        expect(logs^) |> toEqual({j|Error loading $breweryConfig|j})
      }
    )
  }
);