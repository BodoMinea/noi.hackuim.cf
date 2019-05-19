<?php

namespace App\Http\Controllers;

use http\Exception;
use Illuminate\Support\Facades\DB;
use Illuminate\Http\Request;

use Auth;

function generateRandomString($length = 10) {
    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $charactersLength = strlen($characters);
    $randomString = '';
    for ($i = 0; $i < $length; $i++) {
        $randomString .= $characters[rand(0, $charactersLength - 1)];
    }
    return $randomString;
}

class UserController extends Controller
{
    private $UserID;

    /**
     * Create a new controller instance.
     *
     * @return void
     */
    public function __construct()
    {
        //
    }

    public function GetLoginToken()
    {
        if (!$this->UserID)
            return;

        $user = DB::table('users')->where('id', $this)->first();

        return $user->login_token;
    }

    public function GenerateLoginToken()
    {
        if (!$this->UserID)
            return;

        $newToken = generateRandomString(16);
        DB::table('users')
            ->where(['id' => $this->UserID])
            ->update(['login_token' => $newToken]);

        return $newToken;
    }

    public function RegisterUser(Request $request)
    {
        $this->validate($request, [
            'email' => 'required|email|unique:users',
            'password' => 'required',
            'name' => 'required',
            'surname' => 'required',
            'cnp' => 'required|size:13',
            'card_id' => 'integer',
        ]);

        $id = DB::table('users')->insertGetId([
            'email' => $request->input('email'),
            'password' => hash('sha256', $request->input('password')),
            'name' => $request->input('name'),
            'surname' => $request->input('surname'),
            'cnp' => $request->input('cnp'),
            'card_id' => $request->input('card_id')
        ]);

        $this->UserID = $id;

        return response()->json([
            'status' => 'success',
            'login_token' => $this->GenerateLoginToken(),
        ]);
    }

    public function GetMoneyDetails(Request $request) {
        $user = Auth::user();

        $transactions = DB::table('transactions')->join('users', 'users.id', '=', 'transactions.operator_id')->where(['user_id' => $user->id])->orderBy('transactions.id', 'desc')->get();
        $trans_array = [];

        foreach($transactions as $transaction) {
            $trans_array[] = [
                'operator_name' => $transaction->name,
                'amount' => $transaction->amount,
                'timestamp' => strtotime($transaction->date),
            ];
        }

        return response()->json([
            'balance' => $user->balance_cache,
            'uid' => $user->card_id,
            'name' => $user->name,
            'surname' => $user->surname,
            'transactions' => $trans_array,
        ]);
    }

    public function Logout(Request $request) {
        $user = Auth::user();

        $user->where(['id' => $user->id])->update(['login_token' => null]);

        return response()->json([
            'status' => 'success'
        ]);
    }

    public function LoginUser(Request $request) {
        $this->validate($request, [
            'email' => 'required',
            'password' => 'required'
        ]);

        $user_data = DB::table('users')
            ->where([
                'email' => $request->input('email'),
                'password' => hash('sha256', $request->input('password')),
            ])
            ->first();

        if(empty($user_data)) {
            return response([
                'status' => 'bad_auth'
            ], 400);
        }

        $this->UserID = $user_data->id;

        return response()->json([
            'status' => 'success',
            'login_token' => $this->GenerateLoginToken()
        ]);
    }
}
