defmodule TermDemoTest do
  use ExUnit.Case
  doctest TermDemo

  test "greets the world" do
    assert TermDemo.hello() == :world
  end
end
