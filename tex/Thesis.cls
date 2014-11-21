%%
%% This is file `Thesis.cls', based on 'ECSthesis.cls', by Steve R. Gunn
%% generated with the docstrip utility.
%%
%% Created by Steve R. Gunn, modified by Sunil Patel: www.sunilpatel.co.uk

\NeedsTeXFormat{LaTeX2e}[1996/12/01]
\ProvidesClass{Thesis}
              [2007/22/02 v1.0
   LaTeX document class]
\def\baseclass{book}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{\baseclass}}
\def\@checkoptions#1#2{
  \edef\@curroptions{\@ptionlist{\@currname.\@currext}}
  \@tempswafalse
  \@tfor\@this:=#2\do{
    \@expandtwoargs\in@{,\@this,}{,\@curroptions,}
    \ifin@ \@tempswatrue \@break@tfor \fi}
  \let\@this\@empty
  \if@tempswa \else \PassOptionsToClass{#1}{\baseclass}\fi
}
\@checkoptions{11pt}{{10pt}{11pt}{12pt}}
\PassOptionsToClass{a4paper}{\baseclass}
\ProcessOptions\relax
\LoadClass{\baseclass}
\newcommand\bhrule{\typeout{------------------------------------------------------------------------------}}

\newcommand\Declaration[1]{
\btypeout{Declaration of Authorship}
\addtotoc{Declaration of Authorship}
\thispagestyle{plain}
\null\vfil
%\vskip 60\p@
\begin{center}{\huge\bf Declaration of Authorship\par}\end{center}
%\vskip 60\p@
{\normalsize #1}
\vfil\vfil\null
%\cleardoublepage
}

\newcommand\btypeout[1]{\bhrule\typeout{\space #1}\bhrule}
\def\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space \number\year}
\def\hoje{\ifcase\month\or
  Janeiro\or Fevereiro\or Março\or Abril\or Maio\or Junho\or
  Julho\or Agosto\or Setembro\or Outubro\or Novembro\or Dezembro\fi
  \space de \number\year}
% you should not load the inputenc or fontenc package when using xetex!
\usepackage{ifxetex}
\ifxetex
  \usepackage{fontspec}
  \defaultfontfeatures{Ligatures=TeX} % To support LaTeX quoting style
  \setmainfont[
    Ligatures=TeX,
    Path=Fonts/,
    Extension=.otf,
    BoldFont=LinLibertine_RB,
    ItalicFont=LinLibertine_RI,
    BoldItalicFont=LinLibertine_RBI
  ]{LinLibertine_R}
\else
  \usepackage[T1]{fontenc}
  \usepackage[utf8]{inputenc}
\fi
% indent the beginning of every section
\usepackage{indentfirst}
\usepackage{setspace}
\onehalfspacing
%\setlength{\parindent}{0pt}
\setlength{\parindent}{24pt}
%\setlength{\parskip}{2.0ex plus0.5ex minus0.2ex}
\usepackage{vmargin}
\setmarginsrb           { 1.0in}  % left margin
                        { 0.6in}  % top margin
                        { 1.0in}  % right margin
                        { 0.8in}  % bottom margin
                        {  20pt}  % head height
                        {0.25in}  % head sep
                        {   9pt}  % foot height
                        { 0.3in}  % foot sep
\raggedbottom
\setlength{\topskip}{1\topskip \@plus 5\p@}
\doublehyphendemerits=10000       % No consecutive line hyphens.
\brokenpenalty=10000              % No broken words across columns/pages.
\widowpenalty=9999                % Almost no widows at bottom of page.
\clubpenalty=9999                 % Almost no orphans at top of page.
\interfootnotelinepenalty=9999    % Almost never break footnotes.
\usepackage{fancyhdr}
\lhead[\rm\thepage]{\fancyplain{}{\sl{\rightmark}}}
\rhead[\fancyplain{}{\sl{\leftmark}}]{\rm\thepage}
\chead{}\lfoot{}\rfoot{}\cfoot{}
\pagestyle{fancy}
\renewcommand{\chaptermark}[1]{\btypeout{\thechapter\space #1}\markboth{\@chapapp\ \thechapter\ #1}{\@chapapp\ \thechapter\ #1}}
\renewcommand{\sectionmark}[1]{}
\renewcommand{\subsectionmark}[1]{}
\def\cleardoublepage{\clearpage\if@twoside \ifodd\c@page\else
\hbox{}
\thispagestyle{empty}
\newpage
\if@twocolumn\hbox{}\newpage\fi\fi\fi}
\usepackage{amsmath,amsfonts,amssymb,amscd,amsthm,xspace}
\theoremstyle{plain}
\newtheorem{example}{Example}[chapter]
\newtheorem{theorem}{Theorem}[chapter]
\newtheorem{corollary}[theorem]{Corollary}
\newtheorem{lemma}[theorem]{Lemma}
\newtheorem{proposition}[theorem]{Proposition}
\newtheorem{axiom}[theorem]{Axiom}
%\theoremstyle{definition} % i want definitions to be emph
\newtheorem{definition}[theorem]{Definition}
\theoremstyle{remark}
\newtheorem{remark}[theorem]{Remark}
\usepackage[small,sc,labelfont=bf]{caption}
\setlength{\captionmargin}{20pt}
\renewcommand{\topfraction}{0.85}
\renewcommand{\bottomfraction}{.85}
\renewcommand{\textfraction}{0.1}
\renewcommand{\dbltopfraction}{.85}
\renewcommand{\floatpagefraction}{0.75}
\renewcommand{\dblfloatpagefraction}{.75}
\setcounter{topnumber}{9}
\setcounter{bottomnumber}{9}
\setcounter{totalnumber}{20}
\setcounter{dbltopnumber}{9}
\usepackage{graphicx}
%\usepackage[scriptsize]{subfigure} % obsolete. if needed, use subfig or subcaption
\usepackage{subcaption}
\captionsetup[subfigure]{font=footnotesize}
\usepackage{booktabs}
\usepackage{rotating}
\usepackage{listings}
\usepackage{lstpatch}
\lstset{captionpos=b,
  frame=tb,
  basicstyle=\scriptsize\ttfamily,
  showstringspaces=false,
  keepspaces=true}
\lstdefinestyle{matlab} {
  language=Matlab,
  keywordstyle=\color{blue},
  commentstyle=\color[rgb]{0.13,0.55,0.13}\em,
  stringstyle=\color[rgb]{0.7,0,0} }
\lstdefinestyle{C} {
  language=C,
  tabsize=2,
  breaklines=true,
  breakatwhitespace=true,
  basicstyle={\footnotesize\ttfamily},
  numberstyle=\tiny\color{gray},
  keywordstyle=\color{blue},
  commentstyle=\color[rgb]{0.13,0.55,0.13}\em,
  stringstyle=\color{mauve},
}
\usepackage[pdfpagemode={UseOutlines},bookmarks=true,bookmarksopen=true,
   bookmarksopenlevel=0,bookmarksnumbered=true,hypertexnames=false,
   ,linkcolor={blue},citecolor={blue},urlcolor={red},hidelinks,
   pdfstartview={FitV},unicode,pdfencoding=auto,breaklinks=true]{hyperref}
\pdfstringdefDisableCommands{
   \let\\\space
}
\newcommand*{\supervisor}[1]{\def\supname{#1}}
\newcommand*{\examiner}[1]{\def\examname{#1}}
\newcommand*{\degree}[1]{\def\degreename{#1}}
\newcommand*{\authors}[1]{\def\authornames{#1}}
\newcommand*{\addresses}[1]{\def\addressnames{#1}}
\newcommand*{\university}[1]{\def\univname{#1}}
\newcommand*{\department}[1]{\def\deptname{#1}}
\newcommand*{\group}[1]{\def\groupname{#1}}
\newcommand*{\faculty}[1]{\def\facname{#1}}
\newcommand*{\course}[1]{\def\mei{#1}}
\newcommand*{\city}[1]{\def\cityname{#1}}
\newcommand*{\subject}[1]{\def\subjectname{#1}}
\newcommand*{\keywords}[1]{\def\keywordnames{#1}}
\newcommand*{\documentname}[1]{\def\docname{#1}}
\newcommand*{\oriented}[1]{\def\orientedby{#1}}
\supervisor  {{Professor Manuel Bernardo Barbosa}}
\examiner    {}
\degree      {}
\authors     {}
%\university  {\texorpdfstring{\href{University Web Site URL Here (include http://)}
\university  {{Universidade do Minho}}
%\department  {\texorpdfstring{\href{Department or School Web Site URL Here (include http://)}
\department  {{Departamento de Informática}}
\group       {\texorpdfstring{\href{Research Group Web Site URL Here (include http://)}
                {Research Group Name}}
                {Research Group Name}}
\faculty     {{Escola de Engenharia}}
%\course      {{Master Course in Informatics Engineering}}
\course      {{Mestrado em Engenharia Informática}}
\city        {{Braga}}
\addresses   {}
\subject     {}
\keywords    {}
\documentname{{Dissertação de Mestrado}}
\oriented    {{Trabalho realizado sob orientação de}}
\usepackage{color}
\usepackage{xcolor}
\renewcommand\maketitle{
  \btypeout{Title Page}
  \hypersetup{pdftitle={\@title}}
  \hypersetup{pdfsubject=\subjectname}
  \hypersetup{pdfauthor=\authornames}
  \hypersetup{pdfkeywords=\keywordnames}
  \thispagestyle{empty}
  
  \ifxetex
    \input{Definitions/UMCover}
  \else
    \begin{titlepage}
      \let\footnotesize\small
      \let\footnoterule\relax
      \let \footnote \thanks
      \setcounter{footnote}{0}
      \null\vfil
      %\vskip 60\p@
      \begin{center}
        \setlength{\parskip}{0pt}
        %{\begin{figure}[h]
        %  \begin{center}
        %  \includegraphics{Images/EENG}
        %\end{center}
        %\end{figure}}
        \centering
          \input{Definitions/UMLogo}
          \UMEng\\[2cm]
        {\large \textbf{\UNIVNAME} \par}
        \smallskip
        {\large \FACNAME \par}
        \vfill
        {\huge \bf \@title \par}
        \vfill
        {\LARGE by \par}
        \smallskip
        {\LARGE \authornames \par}
        \vfill
        %{\large A thesis submitted in partial fulfillment for the \par}
        %{\large degree of Doctor of Philosophy \par}
        %{\large Dissertation \par}
        {\large Pre-Dissertation Report \par}
        \bigskip
        {\Large \mei \par}
        \bigskip
        \bigskip
        {\large \textit{Supervised by} \par}
        {\large \supname \par}
        %{\large in the \par}
        %{\large \facname \par}
        %{\large \deptname \par}
        \bigskip
        \bigskip
        \bigskip
        {\Large \@date \par}
        \bigskip
      \end{center}
      \par
      \@thanks
      \vfil\null
    \end{titlepage}
  \fi
  
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
  \cleardoublepage
}

\newenvironment{abstractpage}
{
  \btypeout{Abstract Page}
  \thispagestyle{empty}
  \null\vfil
}
{
  %\vfill
  %\cleardoublepage
  \vfil\vfil\vfil\null
  \cleardoublepage
}
\ifxetex
  % babel equivalent for xetex
  \usepackage{polyglossia}
  \setdefaultlanguage{english}
  \setotherlanguage{portuges}
\else
  % call english in the end
  \usepackage[portuguese,english]{babel}
\fi
\newenvironment{abstract}[1]
  {\bigskip\selectlanguage{#1}%
   \begin{center}
    {\large \bfseries\abstractname}
   \end{center}
  }
  {\par\bigskip}

\addtocounter{secnumdepth}{1}
\setcounter{tocdepth}{5}
\newcounter{dummy}
\newcommand\addtotoc[1]{
\refstepcounter{dummy}
\addcontentsline{toc}{chapter}{#1}}
\renewcommand\tableofcontents{
\btypeout{Table of Contents}
\begin{spacing}{1}{
    \setlength{\parskip}{1pt}
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\contentsname
        \@mkboth{
           \MakeUppercase\contentsname}{\MakeUppercase\contentsname}}
    \@starttoc{toc}
    \if@restonecol\twocolumn\fi
   \cleardoublepage
}\end{spacing}
}
\renewcommand\listoffigures{
\btypeout{List of Figures}
\addtotoc{List of Figures}
\begin{spacing}{1}{
    \setlength{\parskip}{1pt}
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\listfigurename
      \@mkboth{\MakeUppercase\listfigurename}
              {\MakeUppercase\listfigurename}}
    \@starttoc{lof}
    \if@restonecol\twocolumn\fi
    \cleardoublepage
}\end{spacing}
}
\renewcommand\listoftables{
\btypeout{List of Tables}
\addtotoc{List of Tables}
\begin{spacing}{1}{
    \setlength{\parskip}{1pt}
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\listtablename
      \@mkboth{
          \MakeUppercase\listtablename}{\MakeUppercase\listtablename}}
    \@starttoc{lot}
    \if@restonecol\twocolumn\fi
    \cleardoublepage
}\end{spacing}
}
\newcommand\listsymbolname{Abbreviations}
\usepackage{longtable}
\newcommand\listofsymbols[2]{
\btypeout{\listsymbolname}
\addtotoc{\listsymbolname}
    \chapter*{\listsymbolname
      \@mkboth{
          \MakeUppercase\listsymbolname}{\MakeUppercase\listsymbolname}}
\begin{longtable}[c]{#1}#2\end{longtable}\par
    \cleardoublepage
}
\newcommand\listconstants{Physical Constants}
% \usepackage{longtable} % duplicate
\newcommand\listofconstants[2]{
\btypeout{\listconstants}
\addtotoc{\listconstants}
    \chapter*{\listconstants
      \@mkboth{
          \MakeUppercase\listconstants}{\MakeUppercase\listconstants}}
\begin{longtable}[c]{#1}#2\end{longtable}\par
    \cleardoublepage
}
\newcommand\listnomenclature{Symbols}
\usepackage{longtable}
\newcommand\listofnomenclature[2]{
\btypeout{\listnomenclature}
\addtotoc{\listnomenclature}
    \chapter*{\listnomenclature
      \@mkboth{
          \MakeUppercase\listnomenclature}{\MakeUppercase\listnomenclature}}
\begin{longtable}[c]{#1}#2\end{longtable}\par
    \cleardoublepage
}
\newcommand\acknowledgements[1]{
\btypeout{Acknowledgements}
\addtotoc{Acknowledgements}
%\thispagestyle{plain} % no header or footers
{\huge{\textbf{Acknowledgements}} \par}
{\normalsize #1}
\vfil\vfil\null
\cleardoublepage
}
\newcommand\dedicatory[1]{
\btypeout{Dedicatory}
\thispagestyle{plain}
\null\vfil
\vskip 60\p@
\begin{center}{\Large \sl #1}\end{center}
\vfil\null
\cleardoublepage
}
\renewcommand\backmatter{
  \if@openright
    \cleardoublepage
  \else
    \clearpage
  \fi
  \addtotoc{\bibname}
  \btypeout{\bibname}
  \@mainmatterfalse}
%%%%%%%%%%%%%%%%% My own usepackages
% Needed for the "comment" environment to make LaTeX comments
\usepackage{verbatim}
% Allows "\bvec{}" and "\buvec{}" for "blackboard" style bold vectors in maths
\usepackage{vector}
% Todos
\usepackage{todonotes}
% set default todo style to fancyline
\presetkeys{todonotes}{fancyline,size=\tiny}{}

% show frames around the margins
%\usepackage[showframe,pass]{geometry}
\usepackage[pass]{geometry}
% nomenclature and glossary
\usepackage[norefpage]{nomencl}
\renewcommand{\nomname}{Abbreviations}
% Abbreviation in bold
\renewcommand{\nomlabel}[1]{\textbf{#1}}
% no extra skip between entries
\setlength{\nomitemsep}{-\parsep}
% costumize the rightmark and leftmark from fancyhdr
\renewcommand{\chaptermark}[1]{ \markboth{\MakeUppercase{\thechapter.\ #1}}{}}
\renewcommand{\sectionmark}[1]{ \markright{\thesection.\ #1}{ }}

\makenomenclature

\usepackage{comment}

% include my new commands
\usepackage{Commands}
% To include pdfs
%\usepackage{pdfpages}
% algorithms
\usepackage{algpseudocode} % for algorithms
\usepackage{algorithm} % for algorithms
\usepackage{algorithmicx} % for algorithms
\usepackage{enumerate}
% plot graphs from dat files: http://tex.stackexchange.com/a/54910
\usepackage{pgfplots}
\pgfplotsset{
  compat=newest,
  % Style to select only points from #1 to #2 (inclusive)
  % http://tex.stackexchange.com/a/199396
  select coords between index/.style 2 args={
    x filter/.code={
        \ifnum\coordindex<#1\def\pgfmathresult{}\fi
        \ifnum\coordindex>#2\def\pgfmathresult{}\fi
    }
  },
}
\usepackage{multirow}
\endinput
%%
%% End of file `Thesis.cls'.
