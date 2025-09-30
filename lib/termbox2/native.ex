defmodule Termbox2.Native do
  @moduledoc """
  Low-level bindings to the Termbox2 C library exposed through a Native
  Implemented Function (NIF).

  Each function maps closely to its Termbox2 counterpart and either returns
  `:ok`, `{:ok, value}`, `:timeout`, or `{:error, code}` depending on the C
  return value. Most functions in this module perform no additional safety
  checks, so callers should familiarize themselves with the Termbox2 API before
  use. See the [`Termbox2` documentation](https://github.com/termbox/termbox2)
  for background on modes, attributes, and error codes.
  """

  @typedoc "Signed terminal coordinate measured in character cells."
  @type coord :: integer()

  @typedoc "Bitmask or palette value understood by Termbox2."
  @type attr :: non_neg_integer()

  @typedoc "Error code returned by the Termbox2 library."
  @type error_code :: integer()

  @typedoc "Result for functions that only signal success or error."
  @type result :: :ok | {:error, error_code}

  @typedoc "Result for functions that return a value on success."
  @type result(value) :: {:ok, value} | {:error, error_code}

  @typedoc "Representation of a Termbox2 cell."
  @type cell :: %{
          required(:ch) => non_neg_integer(),
          required(:fg) => attr(),
          required(:bg) => attr(),
          optional(:ech) => [non_neg_integer()],
          optional(:nech) => non_neg_integer(),
          optional(:cech) => non_neg_integer()
        }

  @typedoc "Snapshot of the screen buffer returned by `cell_buffer/0`."
  @type cell_buffer :: %{
          required(:width) => non_neg_integer(),
          required(:height) => non_neg_integer(),
          required(:cells) => [cell()]
        }

  @typedoc "Termbox2 key event."
  @type key_event :: %{
          required(:type) => :key,
          required(:mod) => non_neg_integer(),
          required(:key) => non_neg_integer(),
          required(:ch) => non_neg_integer()
        }

  @typedoc "Termbox2 mouse event."
  @type mouse_event :: %{
          required(:type) => :mouse,
          required(:mod) => non_neg_integer(),
          required(:key) => non_neg_integer(),
          required(:x) => coord(),
          required(:y) => coord()
        }

  @typedoc "Termbox2 resize event."
  @type resize_event :: %{
          required(:type) => :resize,
          required(:w) => non_neg_integer(),
          required(:h) => non_neg_integer()
        }

  @typedoc "Union of possible event payloads produced by `peek_event/1` and `poll_event/0`."
  @type event :: key_event() | mouse_event() | resize_event() | map()

  @typedoc "Return value of `get_fds/0`."
  @type fds :: %{required(:ttyfd) => integer(), required(:resizefd) => integer()}

  @typedoc "Return value of UTF-8 decoding helper."
  @type utf8_decode :: %{
          required(:len) => non_neg_integer(),
          optional(:codepoint) => non_neg_integer()
        }

  @typedoc "Return value of UTF-8 encoding helper."
  @type utf8_encode :: %{required(:len) => non_neg_integer(), required(:string) => binary()}

  @on_load :load_nif

  @doc """
  Loads the NIF from the application's private directory.

  This is invoked automatically when the module is first referenced.
  """
  @spec load_nif() :: :ok
  def load_nif do
    priv = :code.priv_dir(:term_demo)
    :erlang.load_nif(:filename.join(priv, ~c"termbox_nif"), 0)
  end

  @doc """
  Initializes Termbox2 using the default terminal (`tb_init`).

  ## Examples

      Termbox2.Native.init()
      #=> :ok
  """
  @spec init() :: result()
  def init, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Initializes Termbox2 using a specific tty path (`tb_init_file`).

  ## Examples

      Termbox2.Native.init_file("/dev/tty")
      #=> :ok
  """
  @spec init_file(binary() | iodata()) :: result()
  def init_file(_path), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Initializes Termbox2 using an already-opened file descriptor (`tb_init_fd`).
  """
  @spec init_fd(integer()) :: result()
  def init_fd(_fd), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Initializes Termbox2 with separate read and write descriptors (`tb_init_rwfd`).
  """
  @spec init_rwfd(integer(), integer()) :: result()
  def init_rwfd(_rfd, _wfd), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Shuts down Termbox2 (`tb_shutdown`).
  """
  @spec shutdown() :: result()
  def shutdown, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the current terminal width (`tb_width`).
  """
  @spec width() :: integer()
  def width, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the current terminal height (`tb_height`).
  """
  @spec height() :: integer()
  def height, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Clears the back buffer using the currently configured clear attributes (`tb_clear`).
  """
  @spec clear() :: result()
  def clear, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Sets the attributes used by subsequent `clear/0` calls (`tb_set_clear_attrs`).
  """
  @spec set_clear_attrs(attr(), attr()) :: result()
  def set_clear_attrs(_fg, _bg), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Flushes the back buffer to the terminal (`tb_present`).
  """
  @spec present() :: result()
  def present, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Forces a full redraw by clearing the front buffer (`tb_invalidate`).
  """
  @spec invalidate() :: result()
  def invalidate, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Moves the cursor to the given coordinates (`tb_set_cursor`).

  Pass `{-1, -1}` to hide the cursor without altering the input mode.
  """
  @spec set_cursor(coord(), coord()) :: result()
  def set_cursor(_x, _y), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Hides the cursor (`tb_hide_cursor`).
  """
  @spec hide_cursor() :: result()
  def hide_cursor, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Writes a single cell to the back buffer (`tb_set_cell`).
  """
  @spec set_cell(coord(), coord(), non_neg_integer(), attr(), attr()) :: result()
  def set_cell(_x, _y, _ch, _fg, _bg), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Writes a grapheme cluster to the back buffer (`tb_set_cell_ex`).

  The third argument is a list of Unicode codepoints. Use an empty list to clear
  any existing extended grapheme data while keeping the base codepoint.
  """
  @spec set_cell_ex(coord(), coord(), [non_neg_integer()], attr(), attr()) :: result()
  def set_cell_ex(_x, _y, _cluster, _fg, _bg), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Appends a single combining codepoint to the grapheme stored at `{x, y}` (`tb_extend_cell`).
  """
  @spec extend_cell(coord(), coord(), non_neg_integer()) :: result()
  def extend_cell(_x, _y, _codepoint), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Reads a cell from either the back or front buffer (`tb_get_cell`).

  Pass `1` for the `back` argument to read from the back buffer.
  """
  @spec get_cell(coord(), coord(), integer()) :: result(cell())
  def get_cell(_x, _y, _which), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Sets the input mode and returns the active mode (`tb_set_input_mode`).
  """
  @spec set_input_mode(integer()) :: result(integer())
  def set_input_mode(_mode), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Sets the output mode and returns the active mode (`tb_set_output_mode`).
  """
  @spec set_output_mode(integer()) :: result(integer())
  def set_output_mode(_mode), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Waits up to `timeout_ms` milliseconds for an event (`tb_peek_event`).

  Returns `:timeout` when no event was available within the interval.
  """
  @spec peek_event(non_neg_integer()) :: :timeout | result(event())
  def peek_event(_timeout_ms), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Blocks until the next event is available (`tb_poll_event`).
  """
  @spec poll_event() :: result(event())
  def poll_event, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the file descriptors Termbox2 uses for input and resize notifications (`tb_get_fds`).
  """
  @spec get_fds() :: result(fds())
  def get_fds, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Renders a UTF-8 string starting at `{x, y}` (`tb_print`).
  """
  @spec print(coord(), coord(), attr(), attr(), iodata()) :: result()
  def print(_x, _y, _fg, _bg, _text), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Renders a UTF-8 string and returns its rendered width (`tb_print_ex`).
  """
  @spec print_ex(coord(), coord(), attr(), attr(), iodata()) :: result(non_neg_integer())
  def print_ex(_x, _y, _fg, _bg, _text), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Formats and prints a string using `:io_lib.format/2` arguments, then delegates to `printf/5`.
  """
  @spec printf(coord(), coord(), attr(), attr(), binary(), list()) :: result()
  def printf(x, y, fg, bg, format, args) do
    format
    |> :io_lib.format(args)
    |> IO.iodata_to_binary()
    |> printf(x, y, fg, bg)
  end

  @doc """
  Prints a UTF-8 string produced from an iodata binary (`tb_printf`).
  """
  @spec printf(coord(), coord(), attr(), attr(), iodata()) :: result()
  def printf(_x, _y, _fg, _bg, _format), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Formats and prints a string while also returning its rendered width (`tb_printf_ex`).
  """
  @spec printf_ex(coord(), coord(), attr(), attr(), binary(), list()) :: result(non_neg_integer())
  def printf_ex(x, y, fg, bg, format, args) do
    format
    |> :io_lib.format(args)
    |> IO.iodata_to_binary()
    |> printf_ex(x, y, fg, bg)
  end

  @doc """
  Prints an iodata string and returns its rendered width (`tb_printf_ex`).
  """
  @spec printf_ex(coord(), coord(), attr(), attr(), iodata()) :: result(non_neg_integer())
  def printf_ex(_x, _y, _fg, _bg, _format), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Sends raw bytes to the terminal output buffer (`tb_send`).
  """
  @spec send(iodata()) :: result()
  def send(_iodata), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Formats and sends bytes using `:io_lib.format/2`, then delegates to `sendf/1`.
  """
  @spec sendf(binary(), list()) :: result()
  def sendf(format, args) do
    format
    |> :io_lib.format(args)
    |> IO.iodata_to_binary()
    |> sendf()
  end

  @doc """
  Sends a formatted UTF-8 string to the terminal (`tb_sendf`).
  """
  @spec sendf(iodata()) :: result()
  def sendf(_format), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Updates the legacy extract callbacks (`tb_set_func`). Only `nil` is supported for clearing callbacks.
  """
  @spec set_func(integer(), nil) :: result()
  def set_func(_type, _fun), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the length in bytes of a UTF-8 sequence given its first byte (`tb_utf8_char_length`).
  """
  @spec utf8_char_length(non_neg_integer()) :: integer()
  def utf8_char_length(_byte), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Decodes a UTF-8 sequence (`tb_utf8_char_to_unicode`). The result map always contains `:len` and may include `:codepoint`.
  """
  @spec utf8_char_to_unicode(iodata()) :: result(utf8_decode())
  def utf8_char_to_unicode(_bytes), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Encodes a Unicode codepoint into UTF-8 (`tb_utf8_unicode_to_char`).
  """
  @spec utf8_unicode_to_char(non_neg_integer()) :: result(utf8_encode())
  def utf8_unicode_to_char(_codepoint), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the last `errno` captured by Termbox2 (`tb_last_errno`).
  """
  @spec last_errno() :: integer()
  def last_errno, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Converts a Termbox2 error code into a human-readable message (`tb_strerror`).
  """
  @spec strerror(error_code()) :: result(binary())
  def strerror(_code), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the back-buffer cells along with width and height (`tb_cell_buffer`).
  """
  @spec cell_buffer() :: result(cell_buffer())
  def cell_buffer, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Indicates whether Termbox2 was compiled with truecolor support (`tb_has_truecolor`).
  """
  @spec has_truecolor() :: boolean()
  def has_truecolor, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Indicates whether extended grapheme clusters are enabled (`tb_has_egc`).
  """
  @spec has_egc() :: boolean()
  def has_egc, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the bit-width of attribute storage (`tb_attr_width`).
  """
  @spec attr_width() :: non_neg_integer()
  def attr_width, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the linked Termbox2 version string (`tb_version`).
  """
  @spec version() :: binary()
  def version, do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Tests whether a codepoint is printable according to Termbox2 (`tb_iswprint`).
  """
  @spec iswprint(non_neg_integer()) :: boolean()
  def iswprint(_codepoint), do: :erlang.nif_error(:nif_not_loaded)

  @doc """
  Returns the display width of a codepoint (`tb_wcwidth`).
  """
  @spec wcwidth(non_neg_integer()) :: integer()
  def wcwidth(_codepoint), do: :erlang.nif_error(:nif_not_loaded)
end
