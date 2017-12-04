open Result;

type brewConfig = {
  cask: list(package),
  brew: list(package)
}
and package = {name: string};

let make = (~brew, ~cask) => {
    cask: List.map((p) => { { name: p } }, cask),
    brew: List.map((p) => { { name: p } }, brew),
};

let add = (brewConfig, formula) => {
  let package = { name: formula };
  {
    ...brewConfig,
    brew: List.append(brewConfig.brew, [package])
  }
};

let toDict = (brewConfig) => {
  let dict = Js.Dict.empty();
  let packages = (packages) =>
    packages
    |> List.map(({ name }) => name)
    |> Array.of_list
    ;
  Js.Dict.set(dict, "cask", Js.Json.stringArray(packages(brewConfig.cask))); 
  Js.Dict.set(dict, "brew", Js.Json.stringArray(packages(brewConfig.brew))); 
  dict
};

let toJson = (brewConfig) => {
  Js.Json.stringifyAny(brewConfig |> toDict);
};

let fromJson = (brewConfig) => {
  let json =
    try (Ok(Js.Json.parseExn(brewConfig))) {
    | _ => Error("Error parsing JSON string")
    };

  let package = (json) =>
    json
    |> Json.Decode.string
    |> (p) => { name: p }
    ;
  let categories = (json) =>
    Json.Decode.{
      brew: json |> field("brew", list(package)),
      cask: json |> field("cask", list(package))
    };

  json
  <$> categories
};


