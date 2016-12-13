const path = require('path');

const PATHS = {
    build: path.resolve(__dirname, 'build'),
    components: path.resolve(__dirname, 'components')
};

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