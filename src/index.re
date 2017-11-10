type system = {
  log: string => unit,
  writeFile: (string, string) => unit
};

[@bs.module "os"] external homedir : unit => string = "";

type command =
  | Help
  | Unknown
  | Init;

type result =
  | Ok(string)
  | Error(string);

let commandToString = (command: command) : string =>
  switch command {
  | Help => "help"
  | Init => "init"
  | Unknown => "unknown"
  };

let breweryConfig = homedir() ++ "/.brewery.json";

let execCommand = (system, (command, options)) =>
  switch command {
  | Help => Ok("here some help")
  | Init =>
    let brewConfig = Js.Json.stringifyAny(Brewconfig.make() |> Brewconfig.toDict);
    switch brewConfig {
    | Some(s) =>
      system.writeFile(breweryConfig, s);
      Ok(".brewery.json created")
    | None => Error("unable to create initial brewery.json")
    }
  | Unknown =>
    Js.log("I don't know " ++ (commandToString(command) ++ " command"));
    Error(commandToString(command))
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

let run = (system, stdin) =>
  parseArguments(stdin)
  |> getCommand
  |> execCommand(system)
  |> (
    (res) =>
      switch res {
      | Ok(resText) => system.log(resText)
      | Error(resText) => system.log(resText)
      }
  );

let system = {log: Js.log, writeFile: Node_fs.writeFileAsUtf8Sync};

let () = run(system, Node_process.argv);
