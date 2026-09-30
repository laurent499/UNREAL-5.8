import os
import json
import re
import glob
import locale

def main():
    # Définir la locale sur UTF-8 (à adapter selon ton OS : 'fr_FR.UTF-8' par ex.)
    locale.setlocale(locale.LC_ALL, 'fr_FR.UTF-8')
    
    # Récupérer tous les PNG
    png_files = glob.glob("*.png")
    
    renamed_files = {}
    
    for file in png_files:
        filename_without_ext = os.path.splitext(file)[0]
        
        # Supprimer les numéros au début et les underscores
        new_name = re.sub(r'^\d+_', '', filename_without_ext)
        new_name = new_name.replace('_', ' ')
        
        new_filename = f"{new_name}.png"
        
        try:
            os.rename(file, new_filename)
            print(f"Renamed: {file} -> {new_filename}")
            renamed_files[new_name] = new_filename
        except Exception as e:
            print(f"Error renaming {file}: {e}")
    
    # Tri prenant en compte les caractères accentués
    sorted_names = sorted(renamed_files.keys(), key=locale.strxfrm)
    
    json_data = {"Photos": {}}
    
    for i, name in enumerate(sorted_names, 1):
        json_data["Photos"][name] = i
    
    with open("photos.json", "w", encoding="utf-8") as json_file:
        json.dump(json_data, json_file, indent=4, ensure_ascii=False)
    
    print(f"\nCreated photos.json with {len(sorted_names)} entries.")

if __name__ == "__main__":
    main()
