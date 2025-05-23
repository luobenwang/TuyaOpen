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

project = 'TuyaOpen Development Guide'
copyright = u'2021-{}, Tuya Inc'.format(current_year)
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

language = 'en'
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
    'prev_next_buttons_location': 'both',
    'style_external_links': False,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False,
    'collapse_navigation': False,
    'logo_only': True,
    'body_max_width': None,
    'sidebarwidth': '25%',
}

html_js_files = [
    'js/custom.js',
    'js/include_html.js'
]

html_context = {
    "display_github": True,
    "github_user": 'tuya',
    "github_repo": 'TuyaOpen',
    "github_version": 'master',
    "conf_py_path": '/docs/en/',
}

highlight_language = 'c'
primary_domain = 'c'

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