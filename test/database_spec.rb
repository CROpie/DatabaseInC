
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
    it 'inserts one row of data into memory' do
      result = run_script([
        "insert Yo, Peanut!",
        "select",
        "exit"
      ])
      expect(result).to match_array([
        "Executed.",
        "0) Yo, Peanut!",
        "goodbyte",
      ])
    end

    it 'inserts two rows of data into memory, then select both in one operation' do
      result = run_script([
        "insert Yo, Peanut!",
        "insert Hi, how is it going?",
        "select *",
        "exit"
      ])
      expect(result).to match_array([
        "Executed.",
        "Executed.",
        "0) Yo, Peanut!",
        "1) Hi, how is it going?",
        "goodbyte",
    ])
  end

    it 'inserts two rows of data into memory, can select one of them' do
      result = run_script([
        "insert Yo, Peanut!",
        "insert Hi, how is it going?",
        "select 1",
        "exit"
      ])

      expect(result).to match_array([
        "Executed.",
        "Executed.",
        "1) Hi, how is it going?",
        "goodbyte",
      ])
    end

    it 'inserts one row of data into memory, then delete it' do
      result = run_script([
        "insert Yo, Peanut!",
        "select",
        "delete 0",
        "select",
        "exit"
      ])
      expect(result).to match_array([
        "Executed.",
        "0) Yo, Peanut!",
        "Executed.",
        "goodbyte",
      ])
    end

    it 'inserts two rows of data into memory, then delete both in one operation' do
      result = run_script([
        "insert Yo, Peanut!",
        "insert Hi, how is it going?",
        "select",
        "delete *",
        "select",
        "exit"
      ])
      expect(result).to match_array([
        "Executed.",
        "Executed.",
        "0) Yo, Peanut!",
        "1) Hi, how is it going?",
        "Executed.",
        "goodbyte",
      ])
    end

    # Unhappy
    it 'handles trying to delete a non-existant record' do
      result = run_script([
        "delete 0",
        "exit"
      ])
      expect(result).to match_array([
        "ERROR: not a valid command.",
        "goodbyte",
      ])
    end

    it 'handles trying to select a non-existant record' do
      result = run_script([
        "select 0",
        "exit"
      ])
      expect(result).to match_array([
        "ERROR: not a valid command.",
        "goodbyte",
      ])
    end

end
