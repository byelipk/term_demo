defmodule TermDemo do
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

  def demo(message \\ "Hello from Elixir + termbox2!") do
    try do
      :ok = init()
      :ok = clear()
      w = width()
      h = height()
      x = max(0, div(w - String.length(message), 2))
      y = div(h, 2)

      message
      |> String.to_charlist()
      |> Enum.with_index()
      |> Enum.each(fn {ch, i} ->
        # TB_DEFAULT = 0; pass raw ints for attrs; you can expose a constants map if you like.
        :ok = set_cell(x + i, y, ch, 0, 0)
      end)

      :ok = present()

      case poll_event() do
        {:ok, %{type: :key}} -> :ok
        _ -> :ok
      end
    after
      :ok = shutdown()
    end
  end
end
