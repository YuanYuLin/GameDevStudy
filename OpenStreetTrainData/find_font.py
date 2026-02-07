import matplotlib.font_manager as fm

# 找出系統中所有可用字體
font_list = fm.findSystemFonts()

# 過濾出包含 "CJK" (中日韓) 或 "Hei" (黑體) 的字體
chinese_fonts = [f for f in font_list if 'CJK' in f or 'Hei' in f or 'Kai' in f]

print("找到的可能中文字體路徑：")
for f in chinese_fonts[:5]: # 只列出前5個
    print(f)
    # 取得該字體在 Matplotlib 中的註冊名稱
    try:
        font_prop = fm.FontProperties(fname=f)
        print(f" -> 可用名稱: {font_prop.get_name()}")
    except:
        pass
