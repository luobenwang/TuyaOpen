# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import os
import sys
import datetime

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
current_year = datetime.datetime.now().year

project = 'TuyaOpen 开发指南'
copyright = u'2021-{}, 杭州涂鸦信息技术有限公司'.format(current_year)
author = 'Tuya'
release = '1.3.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

html_baseurl = 'https://github.com/tuya/TuyaOpen' 

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.todo',
    'myst_parser',
    'sphinx_markdown_tables',
    'sphinx_copybutton',
    'sphinxcontrib.mermaid',
    'sphinx_design',
    'sphinx_sitemap',
    'sphinx_multiversion',
	]

templates_path = ['_templates']
exclude_patterns = []
source_suffix = ['.rst', '.md']

language = 'zh_CN'

# 语法高亮设置
pygments_style = 'friendly'

html_logo = '../images/TuyaOpen.png'

def setup(app):
    app.add_css_file('css/custom.css')

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

html_theme_options = {
    'display_version': True,
    'prev_next_buttons_location': 'both',           # 在文档页面的顶部和底部同时显示 "上一页 / 下一页" 导航按钮。
    'style_external_links': False,                  # 允许外部链接的样式。
    'sticky_navigation': True,                      # 使侧边栏导航菜单在页面滚动时保持固定位置。
    'navigation_depth': 4,                          # 设置侧边栏导航的最大层级深度为 4 级
    'includehidden': True,                          # 禁用导航菜单的自动折叠功能
    'titles_only': False,                           # 禁用页面标题的显示
    'collapse_navigation': False,                   # 禁用导航菜单的自动折叠功能
    'logo_only': True,                              # 仅显示 Logo
    'body_max_width': None,                         # 取消页面宽度限制
    'sidebarwidth': '25%',                          # 调整侧边栏宽度
}

html_js_files = [
    'js/custom.js',  # 如果有自定义 JavaScript 文件
    'js/include_html.js'  # 如果需要引入其他 JavaScript 文件
]

# -- GitHub 相关配置 ---------------------------------------------------
# 获取当前构建版本（sphinx-multiversion提供）
current_version = os.environ.get('smv_current_version', 'master')

html_context = {
    # GitHub 仓库设置（必需）
    "display_github": True,  # 启用 GitHub 链接
    "github_user": "tuya",   # 组织/用户名
    "github_repo": "TuyaOpen",  # 仓库名
    "github_version": current_version,  # 动态绑定当前版本
    
    # 页面路径配置（自动生成编辑链接）
    "conf_py_path": "/docs/zh/",  # 配置文件的仓库相对路径
}

highlight_language = 'c'  # 默认高亮C语言代码
primary_domain = 'c'      # 主文档域为C语言

# sphinx-multiversion
smv_branch_whitelist = r'^(master|release/.*)$'
smv_remote_whitelist = r'origin'
smv_tag_whitelist = None
smv_outputdir_format = '{ref.name}'

html_sidebars = {
    '**': [
        'versions.html',
    ],
}