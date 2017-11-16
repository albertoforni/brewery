let installBrewScript = {|/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"|};

type system = {
  log: string => unit,
  writeFile: (string, string) => unit,
  exec: string => string
};

[@bs.module "os"] external homedir : unit => string = "";

type command =
  | Help
  | Unknown
  | Init;

type result('a) =
  | Ok('a)
  | Error(string);

let bind = (result, fn) =>
  switch result {
  | Ok(s) => fn(s)
  | Error(s) => Error(s)
  };

let (>>=) = bind;

let commandToString = (command: command) : string =>
  switch command {
  | Help => "help"
  | Init => "init"
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

let ifFn = (condition, thenDo, elseDo) =>
  if (condition()) {
    thenDo()
  } else {
    elseDo()
  };

let commandOfString = (command: string) : command =>
  command
  |> String.lowercase
  |> (
    fun
    | "help" => Help
    | "init" => Init
    | _ => Unknown
  );

let getCommand = (args) =>
  switch args {
  | [command, ...options] => (commandOfString(command), options)
  | _ => (Unknown, [])
  };

let parseArguments = (args: array(string)) : list(string) =>
  Array.sub(args, 2, Array.length(args) - 2) |> Array.to_list;

let run = (system, stdin) => {
  let writeBrewFile = (brewConfig) =>
    switch (Brewconfig.toJson(brewConfig)) {
    | Some(s) =>
      system.writeFile(breweryConfig, s);
      Ok(".brewery.json created")
    | None => Error("unable to create initial brewery.json")
    };

  let getInstalledFormulas = () => {
    let getInstalledFormulas = () => {
      let getInstalledFormulasFor = (command) => {
        let leaves: string = system.exec(command) |> stringOfBuffer;
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
    
  let installBrew = () =>
    tryCatch(
      () => {
        let _ = system.exec(installBrewScript);
        Ok()
      },
      Error("error installing brew")
    );
  let isBrewInstalled = () => {
    let isInstalled = () => {
      let _ = system.exec("brew");
      true
    };
    try (isInstalled()) {
    | _ => false
    }
  };
  let execCommand = ((command, options)) =>
    switch command {
    | Help => Ok("here some help")
    | Init =>
      ifFn(isBrewInstalled, () => Ok(), installBrew) >>= getInstalledFormulas >>= writeBrewFile
    | Unknown =>
      Js.log("I don't know " ++ (commandToString(command) ++ " command"));
      Error(commandToString(command))
    };
  parseArguments(stdin)
  |> getCommand
  |> execCommand
  |> (
    (res) =>
      switch res {
      | Ok(resText) => system.log(resText)
      | Error(resText) => system.log(resText)
      }
  )
};

let system = {
  log: Js.log,
  writeFile: Node_fs.writeFileAsUtf8Sync,
  exec: (command) => Node_child_process.execSync(command, Node_child_process.option())
};

let () = run(system, Node_process.argv);
