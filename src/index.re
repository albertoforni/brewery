open Result;

let installBrewScript = {|/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"|};

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

let breweryConfig = homedir() ++ "/.brewery.json";

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

let writeBrewFile = (writeFile, brewConfig) =>
  switch (Brewconfig.toJson(brewConfig)) {
  | Some(s) =>
    writeFile(breweryConfig, s);
    Ok()
  | None => Error("unable to create initial brewery.json")
  };

let loadBreweryConfig = (readFile) =>
  tryCatch(
    () =>
      readFile(breweryConfig)
      |> String.trim
      |> Brewconfig.fromJson
      |> (
        fun
        | Ok(brewConf) => Ok(brewConf)
        | Error(err) => Error(err)
      ),
    Error({j|Error loading $breweryConfig|j})
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

let installFormula = (exec, args, brewConfig) => {
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
          let _ = exec("brew install " ++ (if (isCask) {{j|cask $formula|j}} else {formula}));
          Ok()
        },
        Error({j|Error installing $formula formula|j})
      )
      <$> (() => Brewconfig.add(brewConfig, isCask, formula))
  )
};

let uninstallFormula = (exec, args, brewConfig) => {
  let getFormula = () =>
    List.hd(args) == "cask" ? (List.nth(args, 1), true) : (List.hd(args), false);
  tryCatch(() => Ok(getFormula()), Error("No formula has been passed"))
  >>= (
    ((formula, isCask)) =>
      tryCatch(
        () => {
          let _ = exec("brew uninstall " ++ (if (isCask) {{j|cask $formula|j}} else {formula}));
          Ok()
        },
        Error({j|Error uninstalling $formula formula|j})
      )
      <$> (() => Brewconfig.remove(brewConfig, isCask, formula))
  )
};

let installBrewIfNeeded = (exec) =>
  if (isBrewInstalled(exec)) {
    Ok()
  } else {
    installBrew(exec)
  };

let execCommand = (system, (command, args)) =>
  switch command {
  | Help =>
    Ok(
      {j|
Hi from brewery 🍻  here some help

init                          - creates the $breweryConfig file with the formulas you have installed in brew and brew cask
install [cask] [formula]      - installs the formula and adds it to the .brewery.json
list                          - shows the installed formulas
uninstall [cask] [formula]    - uninstalls a formula and removes it from .brewery.json
|j}
    )
  | Init =>
    let writeBrewFileIfDoesNotExists = (brewConfig) =>
      if (system.fileExists(breweryConfig)) {
        Error(breweryConfig ++ " exists already")
      } else {
        writeBrewFile(system.writeFile, brewConfig)
      };
    installBrewIfNeeded(system.exec)
    >>= (
      () =>
        getInstalledFormulas(system.exec)
        >>= writeBrewFileIfDoesNotExists
        |> (
          (res) =>
            switch res {
            | Ok () => Ok(".brewery.json created")
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
            | Ok () => Ok(".brewery.json updated")
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
        | Ok () => Ok(".brewery.json updated")
        | Error(err) => Error(err)
        }
    )
  | Unknown =>
    system.log("I don't know " ++ (commandToString(command) ++ " command"));
    Error(commandToString(command))
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