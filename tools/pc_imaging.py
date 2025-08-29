"""Image processing helpers for the converter.
Contains composition and resizing utilities.
"""
from PIL import Image, ImageFilter, ImageDraw, ImageFont
from pc_constants import TARGET_WIDTH, TARGET_HEIGHT

def create_blurred_background(img_rotated):
    # Create blurred background from rotated image
    blur_scale = 0.3
    small_size = (max(1, int(img_rotated.width * blur_scale)), max(1, int(img_rotated.height * blur_scale)))
    img_small = img_rotated.resize(small_size, Image.Resampling.LANCZOS)
    img_blurred = img_small.filter(ImageFilter.GaussianBlur(radius=8))
    img_blurred_large = img_blurred.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.Resampling.LANCZOS)
    overlay = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0))
    return Image.blend(img_blurred_large, overlay, 0.7)

def add_text_to_background(background_img, metadata, x_offset, side='left'):
    """Usa PIL per il rendering del testo nell'area sfocata.
    Considera che l'immagine è stata ruotata di 90° verso sinistra,
    quindi il testo deve essere ruotato di conseguenza per essere leggibile.
    """
    if not metadata:
        return background_img

    # Calcola l'area disponibile
    available_width = int(x_offset) - 20  # Margine di 20px dal bordo
    if available_width <= 40:
        return background_img

    # Prepara i dati del testo
    lines = []
    for key in ('name', 'set', 'year', 'rarity'):
        if key in metadata and metadata[key]:
            lines.append(str(metadata[key]))
    if not lines:
        return background_img

    # Font con dimensioni ottimizzate per l'area
    font_size = min(38, available_width // 5)  # Font ridotto leggermente
    font = None
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", font_size)
    except:
        try:
            font = ImageFont.truetype("arial.ttf", font_size)
        except:
            font = ImageFont.load_default()

    # Crea un'immagine temporanea per disegnare il testo ruotato
    # Dimensioni scambiate perché ruoteremo di 90°
    text_height = available_width
    text_width = TARGET_HEIGHT
    
    # Crea immagine temporanea per il testo
    text_img = Image.new('RGBA', (text_width, text_height), (0, 0, 0, 0))
    text_draw = ImageDraw.Draw(text_img)
    
    # Calcola posizionamento del testo nell'immagine temporanea
    line_height = font_size + 4  # Interlinea ridotta
    total_text_height = len(lines) * line_height
    
    # Posizione di partenza: centrato verticalmente
    start_y = (text_height - total_text_height) // 2
    start_x = 20  # Margine dal bordo
    
    # Disegna ogni linea di testo
    for i, line in enumerate(lines):
        y_pos = start_y + i * line_height
        
        # Tronca il testo se troppo lungo
        while text_draw.textbbox((0, 0), line, font=font)[2] > (text_width - 40) and len(line) > 10:
            line = line[:-4] + "..."
        
        # Disegna bordino nero intorno alle lettere (stroke effect)
        stroke_width = 2
        for dx in range(-stroke_width, stroke_width + 1):
            for dy in range(-stroke_width, stroke_width + 1):
                if dx != 0 or dy != 0:  # Non disegnare al centro
                    text_draw.text((start_x + dx, y_pos + dy), line, font=font, fill=(0, 0, 0, 255))
        
        # Testo principale bianco sopra il bordino
        text_draw.text((start_x, y_pos), line, font=font, fill=(255, 255, 255, 255))

    # Ruota l'immagine del testo di 90° in senso antiorario (counterclockwise)
    # per compensare la rotazione dell'immagine principale
    text_rotated = text_img.rotate(90, expand=True)
    
    # Applica il testo ruotato sull'immagine di background
    img_with_text = background_img.copy()
    
    # Posiziona il testo ruotato nell'area sinistra
    paste_x = 10  # Margine dal bordo sinistro
    paste_y = (TARGET_HEIGHT - text_rotated.height) // 2  # Centrato verticalmente
    
    # Applica il testo con trasparenza
    if text_rotated.mode == 'RGBA':
        img_with_text.paste(text_rotated, (paste_x, paste_y), text_rotated)
    else:
        img_with_text.paste(text_rotated, (paste_x, paste_y))

    return img_with_text


def compose_final_image(img_rotated, metadata=None):
    """Resize rotated image to fit into target and compose final framed image.
    Returns the final PIL.Image and metadata (new_width, new_height, x_offset, y_offset, scale_factor).
    """
    rot_width, rot_height = img_rotated.size
    scale_x = TARGET_WIDTH / rot_width
    scale_y = TARGET_HEIGHT / rot_height
    scale_factor = min(scale_x, scale_y)
    new_width = max(1, int(rot_width * scale_factor))
    new_height = max(1, int(rot_height * scale_factor))
    img_resized = img_rotated.resize((new_width, new_height), Image.Resampling.LANCZOS)

    # Create background from blurred. We paste the card on the right and place text on the left.
    final_img = create_blurred_background(img_rotated)
    x_offset = TARGET_WIDTH - new_width
    # Paste card at right
    y_offset = 0
    final_img.paste(img_resized, (x_offset, y_offset))
    # Compute left margin for text: space to the left of the pasted card
    left_margin = x_offset
    final_img = add_text_to_background(final_img, metadata, left_margin, side='left')

    return final_img, (new_width, new_height, x_offset, y_offset, scale_factor)
