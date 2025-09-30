defmodule Termbox2.Native do
  @moduledoc false
  @on_load :load_nif

  def load_nif do
    priv = :code.priv_dir(:term_demo)
    :erlang.load_nif(:filename.join(priv, ~c"termbox_nif"), 0)
  end

  def init, do: :erlang.nif_error(:nif_not_loaded)
  def shutdown, do: :erlang.nif_error(:nif_not_loaded)
  def clear, do: :erlang.nif_error(:nif_not_loaded)
  def present, do: :erlang.nif_error(:nif_not_loaded)
  def width, do: :erlang.nif_error(:nif_not_loaded)
  def height, do: :erlang.nif_error(:nif_not_loaded)
  def set_cell(_x, _y, _ch, _fg, _bg), do: :erlang.nif_error(:nif_not_loaded)
  def peek_event(_timeout_ms), do: :erlang.nif_error(:nif_not_loaded)
  def poll_event, do: :erlang.nif_error(:nif_not_loaded)
end
