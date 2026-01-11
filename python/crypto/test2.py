


if __name__ == '__main__':
    import re


    def convert_markdown_to_html_with_zoom(content):
        """
        将Markdown格式的图片链接转换为带有缩放样式的HTML img标签。

        :param content: 包含Markdown图片链接的字符串
        :return: 转换后的HTML img标签字符串
        """
        # 正则表达式模式，用于匹配Markdown图片语法 [alt](src)
        pattern = re.compile(r'$(.*?)$$(.*?)$')

        def replacement(match):
            alt_text = match.group(1)
            src = match.group(2)
            return f'<img src="{src}" alt="{alt_text}" style="zoom:50%;"/>'

        # 使用正则表达式进行替换
        modified_content = pattern.sub(replacement, content)
        return modified_content


    # 示例输入字符串
    markdown_content = """
![IMG_0361](G:/LittleGreenHat/assets/IMG_0361-1739803885378-343.PNG)

![IMG_0362](G:/LittleGreenHat/assets/IMG_0362.PNG)

![IMG_0363](G:/LittleGreenHat/assets/IMG_0363.PNG)

![IMG_0365](G:/LittleGreenHat/assets/IMG_0365.PNG)

![IMG_0366](G:/LittleGreenHat/assets/IMG_0366.PNG)

![IMG_0367](G:/LittleGreenHat/assets/IMG_0367.PNG)

![IMG_0368](G:/LittleGreenHat/assets/IMG_0368.PNG)

![IMG_0369](G:/LittleGreenHat/assets/IMG_0369.PNG)

![IMG_0370](G:/LittleGreenHat/assets/IMG_0370.PNG)

![IMG_0371](G:/LittleGreenHat/assets/IMG_0371.PNG)

![IMG_0372](G:/LittleGreenHat/assets/IMG_0372.PNG)

![IMG_0374](G:/LittleGreenHat/assets/IMG_0374.PNG)

![IMG_0375](G:/LittleGreenHat/assets/IMG_0375.PNG)

![IMG_0376](G:/LittleGreenHat/assets/IMG_0376.PNG)

![IMG_0377](G:/LittleGreenHat/assets/IMG_0377.PNG)

![IMG_0378](G:/LittleGreenHat/assets/IMG_0378.PNG)

![IMG_0379](G:/LittleGreenHat/assets/IMG_0379.PNG)

![IMG_0380](G:/LittleGreenHat/assets/IMG_0380.PNG)

![IMG_0381](G:/LittleGreenHat/assets/IMG_0381.PNG)

![IMG_0382](G:/LittleGreenHat/assets/IMG_0382.PNG)

![IMG_0383](G:/LittleGreenHat/assets/IMG_0383.PNG)

![IMG_0384](G:/LittleGreenHat/assets/IMG_0384.PNG)

![IMG_0385](G:/LittleGreenHat/assets/IMG_0385.PNG)

![IMG_0386](G:/LittleGreenHat/assets/IMG_0386.PNG)

![IMG_0387](G:/LittleGreenHat/assets/IMG_0387.PNG)

![IMG_0388](G:/LittleGreenHat/assets/IMG_0388.PNG)

![IMG_0389](G:/LittleGreenHat/assets/IMG_0389.PNG)

![IMG_0390](G:/LittleGreenHat/assets/IMG_0390.PNG)

![IMG_0391](G:/LittleGreenHat/assets/IMG_0391.PNG)

![IMG_0392](G:/LittleGreenHat/assets/IMG_0392.PNG)

![IMG_0393](G:/LittleGreenHat/assets/IMG_0393.PNG)

![IMG_0394](G:/LittleGreenHat/assets/IMG_0394.PNG)

![IMG_0395](G:/LittleGreenHat/assets/IMG_0395.PNG)

![IMG_0396](G:/LittleGreenHat/assets/IMG_0396.PNG)

![IMG_0397](G:/LittleGreenHat/assets/IMG_0397.PNG)

![IMG_0398](G:/LittleGreenHat/assets/IMG_0398.PNG)

![IMG_0399](G:/LittleGreenHat/assets/IMG_0399.PNG)

![IMG_0400](G:/LittleGreenHat/assets/IMG_0400.PNG)

![IMG_0401](G:/LittleGreenHat/assets/IMG_0401.PNG)

![IMG_0402](G:/LittleGreenHat/assets/IMG_0402.PNG)

![IMG_0403](G:/LittleGreenHat/assets/IMG_0403.PNG)

![IMG_0404](G:/LittleGreenHat/assets/IMG_0404.PNG)

![IMG_0405](G:/LittleGreenHat/assets/IMG_0405.PNG)

![IMG_0406](G:/LittleGreenHat/assets/IMG_0406.PNG)

![IMG_0407](G:/LittleGreenHat/assets/IMG_0407.PNG)

![IMG_0408](G:/LittleGreenHat/assets/IMG_0408.PNG)

![IMG_0409](G:/LittleGreenHat/assets/IMG_0409.PNG)

![IMG_0410](G:/LittleGreenHat/assets/IMG_0410.PNG)

![IMG_0411](G:/LittleGreenHat/assets/IMG_0411.PNG)

![IMG_0412](G:/LittleGreenHat/assets/IMG_0412.PNG)

![IMG_0413](G:/LittleGreenHat/assets/IMG_0413.PNG)

![IMG_0414](G:/LittleGreenHat/assets/IMG_0414.PNG)

![IMG_0415](G:/LittleGreenHat/assets/IMG_0415.PNG)

![IMG_0416](G:/LittleGreenHat/assets/IMG_0416.PNG)

![IMG_0417](G:/LittleGreenHat/assets/IMG_0417.PNG)

![IMG_0418](G:/LittleGreenHat/assets/IMG_0418.PNG)

![IMG_0419](G:/LittleGreenHat/assets/IMG_0419.PNG)

![IMG_0420](G:/LittleGreenHat/assets/IMG_0420.PNG)

![IMG_0421](G:/LittleGreenHat/assets/IMG_0421.PNG)

![IMG_0422](G:/LittleGreenHat/assets/IMG_0422.PNG)

![IMG_0423](G:/LittleGreenHat/assets/IMG_0423.PNG)

![IMG_0424](G:/LittleGreenHat/assets/IMG_0424.PNG)

![IMG_0425](G:/LittleGreenHat/assets/IMG_0425.PNG)

![IMG_0426](G:/LittleGreenHat/assets/IMG_0426.PNG)

![IMG_0427](G:/LittleGreenHat/assets/IMG_0427.PNG)

![IMG_0428](G:/LittleGreenHat/assets/IMG_0428.PNG)

![IMG_0429](G:/LittleGreenHat/assets/IMG_0429.PNG)

![IMG_0430](G:/LittleGreenHat/assets/IMG_0430.PNG)

![IMG_0431](G:/LittleGreenHat/assets/IMG_0431.PNG)

![IMG_0432](G:/LittleGreenHat/assets/IMG_0432.PNG)

![IMG_0433](G:/LittleGreenHat/assets/IMG_0433.PNG)

![IMG_0434](G:/LittleGreenHat/assets/IMG_0434.PNG)

![IMG_0435](G:/LittleGreenHat/assets/IMG_0435.PNG)

![IMG_0436](G:/LittleGreenHat/assets/IMG_0436.PNG)

![IMG_0437](G:/LittleGreenHat/assets/IMG_0437.PNG)

![IMG_0438](G:/LittleGreenHat/assets/IMG_0438.PNG)

![IMG_0439](G:/LittleGreenHat/assets/IMG_0439.PNG)

![IMG_0440](G:/LittleGreenHat/assets/IMG_0440.PNG)

![IMG_0441](G:/LittleGreenHat/assets/IMG_0441.PNG)

![IMG_0442](G:/LittleGreenHat/assets/IMG_0442.PNG)

![IMG_0443](G:/LittleGreenHat/assets/IMG_0443.PNG)

![IMG_0444](G:/LittleGreenHat/assets/IMG_0444.PNG)

![IMG_0445](G:/LittleGreenHat/assets/IMG_0445.PNG)

![IMG_0446](G:/LittleGreenHat/assets/IMG_0446.PNG)
    """

    # 调用函数进行转换
    modified_content = convert_markdown_to_html_with_zoom(markdown_content)

    # 输出结果
    print(modified_content)