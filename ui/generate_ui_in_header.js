//This file is meant to be executed from build.sh
const fs = require('fs');

async function main()
{
	let gktBuilderFile = await fs.promises.readFile('ui/gtkbuilder.ui', 'utf-8');
	let headerTemplate = await fs.promises.readFile('ui/header_template.txt', 'utf-8');

	gktBuilderFile = gktBuilderFile.replaceAll('"','\\"');
	gktBuilderFile = gktBuilderFile.replaceAll('\n','\\\n');

	headerTemplate = headerTemplate.replace("{UI_GOES_HERE}", gktBuilderFile);

	await fs.promises.writeFile('ui/gtk_builder_ui.h', headerTemplate);
}

main();
