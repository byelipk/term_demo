defmodule TermDemo do
  alias Termbox2.{Native, ScreenBuffer}

  defdelegate init(), to: Native
  defdelegate shutdown(), to: Native
  defdelegate clear(), to: Native
  defdelegate present(), to: Native
  defdelegate width(), to: Native
  defdelegate height(), to: Native
  defdelegate peek_event(timeout_ms), to: Native
  defdelegate poll_event(), to: Native

  def run(message \\ "Hello, World!") do
    try do
      :ok = init()
      :ok = clear()

      state = %{
        message: String.to_charlist(message),
        width: width(),
        height: height()
      }

      loop(state)
    after
      :ok = shutdown()
    end
  end

  defp loop(state) do
    _ = render(state)

    case poll_event() do
      {:ok, %{type: :key}} ->
        :ok

      {:ok, %{type: :resize, w: w, h: h}} ->
        loop(%{state | width: w, height: h})

      {:ok, _other} ->
        loop(state)

      {:error, _reason} ->
        :ok
    end
  end

  defp render(%{message: message_chars, width: width, height: height} = state) do
    buffer = ScreenBuffer.new(width, height)

    buffer =
      case place_box(width, height, message_chars) do
        {:ok, {box_x, box_y}} ->
          dims = {box_x, box_y, box_width(message_chars), box_height()}
          draw_box(buffer, dims, message_chars)

        :too_small ->
          draw_plain_text(buffer, width, height, message_chars)
      end

    :ok = clear()

    case ScreenBuffer.blit(buffer) do
      :ok -> :ok
      {:error, code} -> IO.warn("term_demo: failed to blit buffer (error #{code})")
    end

    :ok = present()
    state
  end

  defp place_box(width, height, message_chars) do
    message_len = length(message_chars)
    inner_width = message_len + 2
    box_width = inner_width + 2
    box_height = box_height()

    cond do
      width < box_width or height < box_height ->
        :too_small

      true ->
        start_x = div(width - box_width, 2)
        start_y = div(height - box_height, 2)
        {:ok, {start_x, start_y}}
    end
  end

  defp draw_box(buffer, {start_x, start_y, width, height}, message_chars) do
    top_y = start_y
    bottom_y = start_y + height - 1
    mid_y = start_y + div(height - 1, 2)

    buffer
    |> draw_horizontal_line(start_x, top_y, width)
    |> draw_horizontal_line(start_x, bottom_y, width)
    |> draw_vertical_edges(start_x, start_y, width, height)
    |> fill_midrow(start_x, mid_y, message_chars)
  end

  defp draw_horizontal_line(buffer, _start_x, _y, width) when width <= 0, do: buffer

  defp draw_horizontal_line(buffer, start_x, y, width) do
    left = start_x
    right = start_x + width - 1

    buffer = ScreenBuffer.put(buffer, left, y, ?+)
    buffer = if width > 1, do: ScreenBuffer.put(buffer, right, y, ?+), else: buffer

    if right - left > 1 do
      Enum.reduce((left + 1)..(right - 1), buffer, fn x, acc ->
        ScreenBuffer.put(acc, x, y, ?-)
      end)
    else
      buffer
    end
  end

  defp draw_vertical_edges(buffer, start_x, start_y, width, height) do
    top_y = start_y + 1
    bottom_y = start_y + height - 2

    if top_y <= bottom_y do
      Enum.reduce(top_y..bottom_y, buffer, fn y, acc ->
        acc
        |> ScreenBuffer.put(start_x, y, ?|)
        |> ScreenBuffer.put(start_x + width - 1, y, ?|)
      end)
    else
      buffer
    end
  end

  defp fill_midrow(buffer, start_x, y, message_chars) do
    left_pad = start_x + 1
    right_pad = start_x + length(message_chars) + 2

    buffer
    |> ScreenBuffer.put(left_pad, y, ?\s)
    |> ScreenBuffer.put(right_pad, y, ?\s)
    |> put_chars(left_pad + 1, y, message_chars)
  end

  defp draw_plain_text(buffer, width, height, message_chars) do
    start_x = max(0, div(width - length(message_chars), 2))
    start_y = div(height, 2)

    put_chars(buffer, start_x, start_y, message_chars)
  end

  defp put_chars(buffer, start_x, y, chars) do
    {buffer, _} =
      Enum.reduce(chars, {buffer, start_x}, fn ch, {acc, x} ->
        {ScreenBuffer.put(acc, x, y, ch), x + 1}
      end)

    buffer
  end

  defp box_width(message_chars), do: length(message_chars) + 4
  defp box_height, do: 5
end
