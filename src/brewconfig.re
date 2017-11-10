type brewConfig = {
  cask: list(package),
  brew: list(package)
}
and package = {name: string};

let make = () => {
   cask: [],
   brew: []
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
