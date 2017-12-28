open Result;

let installBrewScript = {|/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"|};
let installCaskScript = {|brew tap caskroom/cask|};

type system = {
  log: string => unit,
  writeFile: (string, string) => unit,
  readFile: string => string,
  exec: string => string,
  fileExists: string => bool
};

[@bs.module "os"] external homedir : unit => string = "";

type command =
  | Help
  | Unknown
  | Install
  | Init
  | List
  | Uninstall;

let commandToString = (command: command) : string =>
  switch command {
  | Help => "help"
  | Init => "init"
  | Install => "install"
  | Unknown => "unknown"
  | List => "list"
  | Uninstall => "uninstall"
  };

let breweryfilePath = homedir() ++ "/.breweryfile.json";

let tryCatch = (fn, error) =>
  try (fn()) {
  | _ => error
  };

let commandOfString = (command: string) : command =>
  command
  |> String.lowercase
  |> (
    fun
    | "help" => Help
    | "init" => Init
    | "install" => Install
    | "list" => List
    | "uninstall" => Uninstall
    | _ => Unknown
  );

let getCommandAndArguments = (args) =>
  switch args {
  | [command, ...rest] => (commandOfString(command), rest)
  | _ => (Unknown, [])
  };

let parseArguments = (args: array(string)) : list(string) =>
  Array.sub(args, 2, Array.length(args) - 2) |> Array.to_list;

let writeBrewFile = (writeFile, breweryfile) =>
  switch (Brewconfig.toJson(breweryfile)) {
  | Some(s) =>
    writeFile(breweryfilePath, s);
    Ok()
  | None => Error("unable to create initial brewery.json")
  };

let loadBreweryConfig = (readFile) =>
  tryCatch(
    () =>
      readFile(breweryfilePath)
      |> String.trim
      |> Brewconfig.fromJson
      |> (
        fun
        | Ok(breweryfile) => Ok(breweryfile)
        | Error(err) => Error(err)
      ),
    Error({j|Error loading $breweryfilePath|j})
  );

let getInstalledFormulas = (exec) =>
  tryCatch(
    () => {
      let getInstalledFormulasFor = (command) => {
        let leaves: string = exec(command) |> Utils.stringOfBuffer;
        leaves
        |> String.trim
        |> Js.String.split("\n")
        |> Array.to_list
        |> List.map(fun (s) => (s: string))
      };
      Ok(
        Brewconfig.make(
          ~brew=getInstalledFormulasFor("brew leaves"),
          ~cask=getInstalledFormulasFor("brew cask list")
        )
      )
    },
    Error("error getting installed formulas")
  );

let installBrew = (exec) =>
  tryCatch(
    () => {
      let _ = exec(installBrewScript);
      let _ = exec(installCaskScript);
      Ok()
    },
    Error("error installing brew")
  );

let isBrewInstalled = (exec) => {
  let isInstalled = () => {
    let _ = exec("brew --version");
    true
  };
  try (isInstalled()) {
  | _ => false
  }
};

let installFormula = (exec, args, breweryfile) => {
  let getFormula = () =>
    if (List.hd(args) == "cask") {
      (List.nth(args, 1), true)
    } else {
      (List.hd(args), false)
    };
  tryCatch(() => Ok(getFormula()), Error("No formula has been passed"))
  >>= (
    ((formula, isCask)) =>
      tryCatch(
        () => {
          let _ = exec("brew " ++ (if (isCask) {{j|cask install $formula|j}} else {j|install $formula|j}));
          Ok()
        },
        Error({j|Error installing $formula formula|j})
      )
      <$> (() => Brewconfig.add(breweryfile, isCask, formula))
  )
};

let uninstallFormula = (exec, args, breweryfile) => {
  let getFormula = () =>
    List.hd(args) == "cask" ? (List.nth(args, 1), true) : (List.hd(args), false);
  tryCatch(() => Ok(getFormula()), Error("No formula has been passed"))
  >>= (
    ((formula, isCask)) =>
      tryCatch(
        () => {
          let _ = exec("brew " ++ (if (isCask) {{j|cask uninstall $formula|j}} else {j|uninstall $formula|j}));
          Ok()
        },
        Error({j|Error uninstalling $formula formula|j})
      )
      <$> (() => Brewconfig.remove(breweryfile, isCask, formula))
  )
};

let installBrewIfNeeded = (exec) =>
  if (isBrewInstalled(exec)) {
    Ok()
  } else {
    installBrew(exec)
  };

let rec execCommand = (system, (command, args)) =>
  switch command {
  | Help =>
    Ok(
      {j|
Hi from brewery 🍻  here some help

help                          - shows this output
init                          - creates .breweryfile.json in your HOME folder with the formulas that are currently installed in brew and brew cask
install [cask] [formula]      - installs the formula and adds it to .breweryfile.json
list                          - shows the installed formulas
uninstall [cask] [formula]    - uninstalls a formula and removes it from .breweryfile.json
|j}
    )
  | Init =>
    let writeBreweryfileIfDoesNotExists = (breweryfile) =>
      if (system.fileExists(breweryfilePath)) {
        Error(breweryfilePath ++ " exists already")
      } else {
        writeBrewFile(system.writeFile, breweryfile)
      };
    installBrewIfNeeded(system.exec)
    >>= (
      () =>
        getInstalledFormulas(system.exec)
        >>= writeBreweryfileIfDoesNotExists
        |> (
          (res) =>
            switch res {
            | Ok () => Ok(".breweryfile.json created")
            | Error(err) => Error(err)
            }
        )
    )
  | Install =>
    installBrewIfNeeded(system.exec)
    >>= (
      () =>
        loadBreweryConfig(system.readFile)
        >>= installFormula(system.exec, args)
        >>= writeBrewFile(system.writeFile)
        |> (
          (res) =>
            switch res {
            | Ok () => Ok(".breweryfile.json updated")
            | Error(err) => Error(err)
            }
        )
    )
  | List =>
    loadBreweryConfig(system.readFile)
    >>= (
      (breweryConfig) => {
        let breweryConfigRes = Brewconfig.toJson(breweryConfig);
        switch breweryConfigRes {
        | Some(breweryConfig) => Ok(breweryConfig)
        | None => Error({j|"Error converting $breweryConfig to json|j})
        }
      }
    )
  | Uninstall =>
    loadBreweryConfig(system.readFile)
    >>= uninstallFormula(system.exec, args)
    >>= writeBrewFile(system.writeFile)
    |> (
      (res) =>
        switch res {
        | Ok () => Ok(".breweryfile.json updated")
        | Error(err) => Error(err)
        }
    )
  | Unknown =>
    if (List.length(args) == 0) {
      execCommand(system, (Help, args))
    } else {
      system.log("I don't know " ++ (commandToString(command) ++ " command"));
      Error(commandToString(command))
    }
  };

let run = (system, stdin) =>
  parseArguments(stdin)
  |> getCommandAndArguments
  |> execCommand(system)
  |> (
    (res) =>
      switch res {
      | Ok(resText) => system.log(resText)
      | Error(resText) => system.log(resText)
      }
  );

let system = {
  log: Js.log,
  writeFile: Node_fs.writeFileAsUtf8Sync,
  readFile: Node_fs.readFileAsUtf8Sync,
  exec: (command) => Node_child_process.execSync(command, Node_child_process.option()),
  fileExists: (filePath) =>
    tryCatch(
      () => {
        let _ = Node_fs.readFileAsUtf8Sync(filePath);
        true
      },
      false
    )
};

let () = run(system, Node_process.argv);