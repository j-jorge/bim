file(READ ${POT_FILE} pot_file_content)
string(
  REGEX REPLACE
  "\"POT-Creation-Date:[^\"]+\\n\"\n"
  ""
  cleaned_up_content
  "${pot_file_content}"
)
file(WRITE ${POT_FILE} "${cleaned_up_content}")
