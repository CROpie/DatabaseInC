# warning: match_array is order insensitive. Could use contain_exactly to check the order. 

RSpec.describe 'database' do
    def run_script(commands)
      raw_output = nil
      IO.popen("./build/db", "r+") do |pipe|
        commands.each do |command|
          pipe.puts command
        end
  
        pipe.close_write
  
    #read entire output
        raw_output = pipe.gets(nil)
      end
      raw_output.split("\n")
    end

    # Unhappy path tests
    it 'handles random input' do
      result = run_script([
        "hello there",
        "exit"
      ])
      expect(result).to match_array([
        "unrecognized command",
        "goodbyte"
      ])
    end

    it 'handles input that is too long' do
      result = run_script([
        "this sentence is over 25 characters which is the limit for this iteration",
      ])
      expect(result).to contain_exactly(
        "Failed to get input",
        "Input was too long and got cut off.",
      )
    end

    it 'handles no input' do
      result = run_script([
        "",
        "exit"
      ])
      expect(result).to match_array([
        "unrecognized command",
        "goodbyte",
      ])
    end
end
