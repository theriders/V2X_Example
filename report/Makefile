opts := --check en --ignore sh:nobreak --dict dico.txt

all:
	-rm eel5632_report_Autogators.aux
	-rm eel5632_report_Autogators.bbl
	-rm eel5632_report_Autogators.blg
	pdflatex eel5632_report_Autogators
	bibtex   eel5632_report_Autogators
	pdflatex eel5632_report_Autogators
	pdflatex eel5632_report_Autogators
	-textidote --output html ${opts} eel5632_report_Autogators.tex > report.html

update:
	cd .. && git add report; git commit -m "$(message)"; git push
