<?php

/*
|--------------------------------------------------------------------------
| Application Routes
|--------------------------------------------------------------------------
|
| Here is where you can register all of the routes for an application.
| It is a breeze. Simply tell Lumen the URIs it should respond to
| and give it the Closure to call when that URI is requested.
|
*/

$router->get('/', function () use ($router) {
    return $router->app->version();
});

/*
$router->get('user/{id}', function ($id) {
   /*$results = DB::select("SELECT * FROM users");
   print_r($results);
   // return $router->app->version();
   // return response()->json(['name' => 'Abigail', 'state' => 'CA']);
});*/


$router->get('/post/{id}', ['middleware' => 'auth', function (Request $request, $id) {
    $user = Auth::user();

    $user = $request->user();

    //
}]);


$router->post('/user/register', [
    'as' => 'register', 'uses' => 'UserController@RegisterUser'
]);

$router->post('/user/login', [
    'as' => 'register', 'uses' => 'UserController@LoginUser'
]);

$router->group(['middleware' => 'auth'], function () use ($router) {
    $router->get('/user/money', [
        'as' => 'balance',
        'uses' => 'UserController@GetMoneyDetails',
    ]);

    $router->post('/user/logout', [
        'as' => 'logout',
        'uses' => 'UserController@Logout',
    ]);
});

$router->group(['middleware' => 'auth'], function () use ($router) {
    $router->post('/transaction/create', [
        'as' => 'balance',
        'uses' => 'TransactionController@Create',
    ]);
});

