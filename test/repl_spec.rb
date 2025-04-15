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
  
    # Happy path tests
    it 'goes down the insert path' do
      result = run_script([
        "insert Yo, Peanut!",
        "exit"
      ])
      expect(result).to match_array([
        "db > running insert",
        "db > goodbyte",
      ])
    end

    it 'goes down the select path' do
      result = run_script([
        "select *",
        "exit"
      ])
      expect(result).to match_array([
        "db > running select",
        "db > goodbyte",
      ])
    end

    # Unhappy path tests
    it 'handles random input' do
      result = run_script([
        "hello there",
        "exit"
      ])
      expect(result).to match_array([
        "db > unrecognized command",
        "db > goodbyte"
      ])
    end

    it 'handles input that is too long' do
      result = run_script([
        "this sentence is over 25 characters which is the limit for this iteration",
      ])
      expect(result).to contain_exactly(
        "Failed to get input",
        "db > Input was too long and got cut off.",
      )
    end

    it 'handles no input' do
      result = run_script([
        "",
        "exit"
      ])
      expect(result).to match_array([
        "db > unrecognized command",
        "db > goodbyte",
      ])
    end
end
