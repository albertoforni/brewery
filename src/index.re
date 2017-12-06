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
  | Init;

let commandToString = (command: command) : string =>
  switch command {
  | Help => "help"
  | Init => "init"
  | Install => "install"
  | Unknown => "unknown"
  };

let breweryConfig = homedir() ++ "/.brewery.json";

let tryCatch = (fn, error) =>
  try (fn()) {
  | _ => error
  };

let stringOfBuffer: string => string = [%bs.raw
  {|
  function stringOfBuffer (buffer) { return buffer.toString(); }
|}
];

let commandOfString = (command: string) : command =>
  command
  |> String.lowercase
  |> (
    fun
    | "help" => Help
    | "init" => Init
    | "install" => Install
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

let loadBrewFile = (readFile, ()) =>
  readFile(breweryConfig)
  |> Brewconfig.fromJson
  |> (
    fun
    | Ok(brewConf) => Ok(brewConf)
    | Error(err) => Error(err)
  );

let getInstalledFormulas = (exec, ()) => {
  let getInstalledFormulas = () => {
    let getInstalledFormulasFor = (command) => {
      let leaves: string = exec(command) |> stringOfBuffer;
      leaves |> Js.String.split("\n") |> Array.to_list |> List.map(fun (s) => (s: string))
    };
    Ok(
      Brewconfig.make(
        ~brew=getInstalledFormulasFor("brew leaves"),
        ~cask=getInstalledFormulasFor("brew cask list")
      )
    )
  };
  tryCatch(getInstalledFormulas, Error("error getting installed formulas"))
};

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

let installFormula = (exec, args, brewConfig) =>
  if (List.length(args) >= 1) {
    let formula = List.hd(args);
    tryCatch(
      () => {
        formula |> ((c) => exec("brew install " ++ c));
        Ok()
      },
      Error("Error installing " ++ formula ++ " formula")
    )
    <$> (() => Brewconfig.add(brewConfig, formula))
  } else {
    Error("No formula has been passed")
  };

let installBrewIfNeeded = (exec) =>
  if (isBrewInstalled(exec)) {
    Ok()
  } else {
    installBrew(exec)
  };

let execCommand = (system, (command, args)) =>
  switch command {
  | Help => Ok("here some help")
  | Init =>
    let writeBrewFileIfDoesNotExists = (brewConfig) =>
      if (system.fileExists(breweryConfig)) {
        Error(breweryConfig ++ " exists already")
      } else {
        writeBrewFile(system.writeFile, brewConfig)
      };
    installBrewIfNeeded(system.exec)
    >>= getInstalledFormulas(system.exec)
    >>= writeBrewFileIfDoesNotExists
    |> (
      (res) =>
        switch res {
        | Ok () => Ok(".brewery.json created")
        | Error(err) => Error(err)
        }
    )
  | Install =>
    installBrewIfNeeded(system.exec)
    >>= loadBrewFile(system.readFile)
    >>= installFormula(system.exec, args)
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