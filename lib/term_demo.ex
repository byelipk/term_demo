defmodule TermDemo do
  alias Termbox2.Native

  defdelegate init(), to: Native
  defdelegate shutdown(), to: Native
  defdelegate clear(), to: Native
  defdelegate present(), to: Native
  defdelegate width(), to: Native
  defdelegate height(), to: Native
  defdelegate set_cell(x, y, ch, fg, bg), to: Native
  defdelegate peek_event(timeout_ms), to: Native
  defdelegate poll_event(), to: Native

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
        evt ->
          dbg(evt)
          :ok
      end
    after
      :ok = shutdown()
    end
  end
end
