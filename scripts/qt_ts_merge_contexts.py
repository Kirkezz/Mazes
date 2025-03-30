import xml.etree.ElementTree as ET
import sys

def merge_contexts(input_file, output_file):
  """Merges all contexts into a single context "SpaceGUI", prioritizing SpaceGUI.

  Args:
    input_file: Path to the input XML file.
    output_file: Path to the output XML file.
  """

  tree = ET.parse(input_file)
  root = tree.getroot()

  new_context = ET.Element("context")
  name_element = ET.Element("name")
  name_element.text = "SpaceGUI"
  new_context.append(name_element)

  seen_sources = set()

  spacegui_context = None
  for context in root.findall("context"):
    if context.find("name").text == "SpaceGUI":
      spacegui_context = context
      break

  if spacegui_context is not None:
    for message in spacegui_context.findall("message"):
      source_text = message.find("source").text
      translation = message.find("translation")
      if translation is not None and translation.get("type") == "vanished":
        del translation.attrib["type"]  # Remove the attribute completely
      new_context.append(message)
      seen_sources.add(source_text)
    root.remove(spacegui_context)

  for context in root.findall("context"):
    for message in context.findall("message"):
      source_text = message.find("source").text
      translation = message.find("translation")
      if translation is not None and translation.get("type") == "vanished":
        del translation.attrib["type"]  # Remove the attribute completely
      if source_text not in seen_sources:
        new_context.append(message)
        seen_sources.add(source_text)
    root.remove(context)

  root.append(new_context)

  tree.write(output_file, encoding="utf-8", xml_declaration=True)

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print("Usage: python script_name.py <input_file>")
    sys.exit(1)

  input_file = sys.argv[1]
  merge_contexts(input_file, input_file)
  print(f"Merged contexts and saved to {input_file}")
