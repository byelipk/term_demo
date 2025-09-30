defmodule Termbox2.Canvas do
  @moduledoc """
  Convenience helpers for writing individual cells with sensible defaults.
  """

  alias Termbox2.Native

  @type attr :: Native.attr()
  @type put_option ::
          {:fg, attr()} | {:bg, attr()} | {:glyph, non_neg_integer()} | {:clamp?, boolean()}
  @type put_result ::
          :ok | :skip | {:ok, %{x: integer(), y: integer()}} | {:error, Native.error_code()}

  @doc """
  Places a glyph at `{x, y}` with optional foreground/background attributes.

  Options:

    * `:fg` - foreground attribute (default `0`)
    * `:bg` - background attribute (default `0`)
    * `:glyph` - overrides the character provided as the third argument
    * `:clamp?` - when `true`, out-of-bounds coordinates are clamped on-screen

  Returns `:skip` when coordinates fall outside the screen and clamping is disabled.
  """
  @spec put_char(integer(), integer(), char | non_neg_integer(), [put_option()]) :: put_result()
  def put_char(x, y, glyph \\ ?\s, opts \\ []) do
    opts = Keyword.merge(default_opts(), opts)
    glyph = Keyword.get(opts, :glyph, glyph)
    fg = Keyword.fetch!(opts, :fg)
    bg = Keyword.fetch!(opts, :bg)

    with {:ok, {nx, ny}} <- maybe_adjust(x, y, opts[:clamp?]),
         result <- Native.set_cell(nx, ny, glyph, fg, bg) do
      case result do
        :ok when nx == x and ny == y -> :ok
        :ok -> {:ok, %{x: nx, y: ny}}
        {:error, code} -> {:error, code}
      end
    else
      :skip -> :skip
      {:error, code} -> {:error, code}
    end
  end

  defp default_opts, do: [fg: 0, bg: 0, clamp?: false]

  defp maybe_adjust(x, y, true) do
    {w, h} = {Native.width(), Native.height()}
    max_x = max(w - 1, 0)
    max_y = max(h - 1, 0)
    {:ok, {x |> min(max_x) |> max(0), y |> min(max_y) |> max(0)}}
  end

  defp maybe_adjust(x, y, false) do
    if inside?(x, y) do
      {:ok, {x, y}}
    else
      :skip
    end
  end

  defp inside?(x, y) do
    x >= 0 and y >= 0 and x < Native.width() and y < Native.height()
  end
end
