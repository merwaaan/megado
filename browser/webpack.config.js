const path = require('path');
const shellPlugin = require('webpack-shell-plugin');

const PATHS = {
    build: path.resolve(__dirname, 'build'),
    components: path.resolve(__dirname, 'components')
};

// TODO add some linter?

module.exports = {
    entry: PATHS.components + '/App.jsx',

    output: {
        path: PATHS.build,
        filename: 'genesis.ui.js'
    },

    resolve: {
        root: path.resolve(__dirname),
        extensions: ['', '.js', '.jsx', '.scss']
    },

    plugins: [
        new shellPlugin({
            // Compile to JS before bundling
            // TODO would be nice to pack the output in the bundle
            onBuildStart: ['make'],
            // Required to re-run the script in watch mode
            dev: false
        })
    ],

    module: {
        loaders: [
            {
                test: /\.jsx?$/, // Process JS and JSX
                loader: 'babel',
                query:
                {
                    presets:['es2015', 'react']
                }
            },
            {
                test: /\.scss$/,
                loaders: ['style', 'css', 'sass']
            }
        ]
    }
}