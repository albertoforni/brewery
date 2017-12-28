open Result;

type breweryfile = {
  cask: list(formula),
  brew: list(formula)
}
and formula = {name: string};

let make = (~brew, ~cask) => {
  cask: List.map((p) => {{name: p}}, cask),
  brew: List.map((p) => {{name: p}}, brew)
};

let add = (breweryfile, isCask, formulaName) => {
  let formula = {name: formulaName};
  {
    cask: isCask && !List.exists(({name}) => name == formulaName, breweryfile.cask) ? breweryfile.cask @ [formula] : breweryfile.cask,
    brew: ! isCask && !List.exists(({name}) => name == formulaName, breweryfile.brew) ? breweryfile.brew @ [formula] : breweryfile.brew
  }
};

let remove = (breweryfile, isCask, formula) => {
  let filterFormula =  List.filter((p) => p.name != formula);
  {
    cask: isCask ? filterFormula(breweryfile.cask) : breweryfile.cask,
    brew: !isCask ? filterFormula(breweryfile.brew) : breweryfile.brew
  }
};

let toDict = (breweryfile) => {
  let dict = Js.Dict.empty();
  let formulas = (formula) => formula |> List.map(({name}) => name) |> Array.of_list;
  Js.Dict.set(dict, "cask", Js.Json.stringArray(formulas(breweryfile.cask)));
  Js.Dict.set(dict, "brew", Js.Json.stringArray(formulas(breweryfile.brew)));
  dict
};

let toJson = (breweryfile) => Utils.jsonStringfy(breweryfile |> toDict);

let fromJson = (breweryfile) => {
  let json =
    try (Ok(Js.Json.parseExn(breweryfile))) {
    | _ => Error("Error parsing JSON string")
    };
  let formula = (json) => json |> Json.Decode.string |> ((p) => {name: p});
  let categories = (json) =>
    Json.Decode.{
      brew: json |> field("brew", list(formula)),
      cask: json |> field("cask", list(formula))
    };
  json <$> categories
};