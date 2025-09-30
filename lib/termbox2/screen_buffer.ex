defmodule Termbox2.ScreenBuffer do
  @moduledoc """
  In-memory representation of a screen frame before flushing to Termbox2.
  """

  alias Termbox2.Native

  @type attr :: Native.attr()
  @type cell :: %{glyph: non_neg_integer(), fg: attr(), bg: attr()}
  @type t :: %__MODULE__{
          width: non_neg_integer(),
          height: non_neg_integer(),
          cells: %{optional({integer(), integer()}) => cell()}
        }

  defstruct width: 0, height: 0, cells: %{}

  @doc """
  Creates a new buffer sized to the provided dimensions (defaults to current terminal size).
  """
  @spec new(non_neg_integer(), non_neg_integer()) :: t()
  def new(width \\ Native.width(), height \\ Native.height()) do
    %__MODULE__{width: width, height: height, cells: %{}}
  end

  @doc """
  Clears the buffer contents.
  """
  @spec clear(t()) :: t()
  def clear(%__MODULE__{} = buffer), do: %{buffer | cells: %{}}

  @doc """
  Stores a cell at `{x, y}` with optional foreground/background attributes.

  Out-of-bounds writes are ignored unless `:clamp?` is true.
  """
  @spec put(t(), integer(), integer(), char | non_neg_integer(), keyword()) :: t()
  def put(%__MODULE__{} = buffer, x, y, glyph, opts \\ []) do
    opts = Keyword.merge([fg: 0, bg: 0, glyph: nil, clamp?: false], opts)
    glyph_code = normalize_glyph(opts[:glyph] || glyph)
    fg = opts[:fg]
    bg = opts[:bg]

    case maybe_adjust(buffer, x, y, opts[:clamp?]) do
      {:ok, {nx, ny}} ->
        cell = %{glyph: glyph_code, fg: fg, bg: bg}
        %{buffer | cells: Map.put(buffer.cells, {nx, ny}, cell)}

      :skip ->
        buffer
    end
  end

  @doc """
  Flushes the buffer to Termbox2 by setting each recorded cell via the NIF.
  """
  @spec blit(t()) :: :ok | {:error, Native.error_code()}
  def blit(%__MODULE__{} = buffer) do
    Enum.reduce_while(buffer.cells, :ok, fn {{x, y}, %{glyph: glyph, fg: fg, bg: bg}}, :ok ->
      case Native.set_cell(x, y, glyph, fg, bg) do
        :ok -> {:cont, :ok}
        {:error, code} -> {:halt, {:error, code}}
      end
    end)
  end

  defp maybe_adjust(%__MODULE__{} = buffer, x, y, true) do
    max_x = max(buffer.width - 1, 0)
    max_y = max(buffer.height - 1, 0)
    {:ok, {clamp(x, 0, max_x), clamp(y, 0, max_y)}}
  end

  defp maybe_adjust(%__MODULE__{} = buffer, x, y, false) do
    if inside?(buffer, x, y) do
      {:ok, {x, y}}
    else
      :skip
    end
  end

  defp inside?(%__MODULE__{} = buffer, x, y) do
    x >= 0 and y >= 0 and x < buffer.width and y < buffer.height
  end

  defp clamp(value, min, max) do
    value
    |> max(min)
    |> min(max)
  end

  defp normalize_glyph(glyph) when is_integer(glyph), do: glyph

  defp normalize_glyph(glyph) when is_binary(glyph) and byte_size(glyph) > 0,
    do: :binary.first(glyph)

  defp normalize_glyph([code | _]), do: code
  defp normalize_glyph(_), do: ?\s
end
