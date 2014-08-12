#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import cairo

WIDTH, HEIGHT = 640, 400

surface = cairo.ImageSurface (cairo.FORMAT_ARGB32, WIDTH, HEIGHT)
cr = cairo.Context (surface)

cr.set_source_rgb(255, 255, 255)
cr.fill()

cr.select_font_face("WenQuanYi Micro Hei", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
cr.set_font_size(20)

cr.move_to(100, 100)
cr.show_text("Hell World")

letters = \
    u"ABCÄÖ€üä$" + \
    u"一三上下不世东个中为丽么之乎乏乐也了予云些亡享人什仅仇从他以们件" + \
    u"企众伙会传伤似但作你佳使依保信倍倒值做停傻像充光兔入全关兵其内再" + \
    u"冰决冷冻出击分切初别到刻前力办功加动助励包北区半单南占卡卫即却原" + \
    u"去又友双发受变口另只可右叶号各同名后向吗否含启呢味命和品哪唯啊喜" + \
    u"器回图土在地坏块垒城域堡塔填墙壁声处复外多夜大天太头奖女她好如妙" + \
    u"始威娃子字存它守完定宝室宫家容宽寂密寒对寻封射将尊小少尚就尼尽屋" + \
    u"屏展山岖岛崎崖崩川左已币布师希带帮常幕干并广应建开弃式张弹强当录" + \
    u"影往径待很得心必快念怪恢恶悬想愉感戏成我或战房所手扩扫找把抓择拿" + \
    u"持指按挑换描提撒撞播支改放效救敬整文斯方旅无既时昏星映是显晚普暂" + \
    u"暗曾最有望木未本术朵机杀杆村束条来板极林果查标树样根格案桢桥检森" + \
    u"模横次欢止正此死殊殿毁每比毫气水池没治法洞活流测深渊游溃满漆漫激" + \
    u"瀑灌火灭点焰然爆爬爱版牢物特犯狂猫献率玩现球生用田电界略疯的皆盘" + \
    u"盲看眺眼着知短石砖破碎碑示祝神禁离秘移稀程穴空穿突窗窟立端第等筑" + \
    u"签简管箱簧粉索纪纵线终经结绕给统继绩续绿置美考者而能脚腥自臭至船" + \
    u"色花菜蛋行补被西要见觉角言计许设试该语说请调谎谢贡财贺赐走起越跃" + \
    u"跑路跳躺车载输辨达过迎运返还这进远迷退送逃逆选途通速道那邪都配醒" + \
    u"里重野金钟钥钮键镜长门闪间阔队降隧雄雪震霉霜静非面音项顺须领题额" + \
    u"风飞餐高魔鱼鹅鹰黑龙遊戲開編關附元選項謝誌離解析度紅利島孵卵後線" + \
    u"動聲樂標鍵盤設搖桿擇檔您語螢長寬白裝發現個密域繼續圖集績壞由時間" + \
    u"事伯住佩來個先克內冒凍利助動區叉吃啟喔塊墟失妮宮層島幣幫幸廢彈後" + \
    u"徑從您情應懷掉損擋攀斷時會朗枚柏梯極橋機檢歡沒洲涼澡爬發盡礙稍紀" + \
    u"紅線繼續羅群與舊藏號裂裡許諾變讓躍輸這進運過還邊開障險離雲電頂頭" + \
    u"幣計風鵝點"

fascent, fdescent, fheight, fxadvance, fyadvance = cr.font_extents()

x, y = (0, fheight)
glyph_w, glyph_h = (20, 20)
cols = WIDTH // glyph_w
rows = HEIGHT // glyph_h

for row in range(rows):
    for col in range(cols):
        idx = row * cols + col
            
        if idx < len(letters):
            letter = letters[idx]
            xbearing, ybearing, width, height, xadvance, yadvance = cr.text_extents(letter)
            print letter

            #cr.move_to(cx + 0.5 - xbearing - width / 2,
            #           0.5 - fdescent + fheight / 2)

            cr.move_to(x + col * glyph_w, 
                       y + row * fheight)
            cr.set_line_width(3.0)
            cr.set_source_rgb(0, 0, 0)
            cr.text_path(letter)
            cr.stroke()

            cr.set_source_rgb(255, 255, 255)
            cr.move_to(x + col * glyph_w, 
                       y + row * fheight)
            cr.show_text(letter)


surface.write_to_png("output.png")

# EOF #
