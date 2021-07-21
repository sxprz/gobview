module Editor : sig
  module ITextModel : sig
    type t = private Ojs.t

    val id : t -> string
  end

  val create_model : value:string -> ?language:string -> unit -> ITextModel.t
    [@@js.global "monaco.editor.createModel"]

  module IViewZone : sig
    type t = private Ojs.t

    val make : ?height_in_lines:float -> after_line_number:int -> dom_node:React.Dom.domElement -> t
      [@@js.builder]
  end

  module IViewZoneChangeAccessor : sig
    type t = private Ojs.t

    val add_zone : t -> IViewZone.t -> string

    val remove_zone : t -> string -> unit
  end

  module IStandaloneCodeEditor : sig
    type t = private Ojs.t

    val set_value : t -> string -> unit [@@js.call]

    val change_view_zones : t -> (IViewZoneChangeAccessor.t -> unit) -> unit
  end

  module IStandaloneEditorConstructionOptions : sig
    type t = { model : ITextModel.t; read_only : bool }
  end

  val create :
    dom_element:React.Dom.domElement ->
    ?options:IStandaloneEditorConstructionOptions.t ->
    unit ->
    IStandaloneCodeEditor.t
    [@@js.global "monaco.editor.create"]
end
