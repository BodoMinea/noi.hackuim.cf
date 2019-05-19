<?php

namespace App\Http\Controllers;

use App\User;
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

class TransactionController extends Controller
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

    public function Create(Request $request) {
        $validator_user = Auth::user();

        $this->validate($request, [
            'uid' => 'required',
            'amount' => 'required',
        ]);

        $user = User::where('card_id', $request->input('uid'))->first();

        $id = DB::table('transactions')->insertGetId([
            'amount' => $request->input('amount'),
            'transaction_type' => $request->input('amount') <= 0 ? 'PAYMENT' : 'TOPUP',
            'operator_id' => $validator_user->id,
            'user_id' => $user->id,
        ]);

        $new_balance = $user->balance_cache + $request->input('amount');
        if($new_balance < 0)
            return "-999";

        $user->where(['id' => $user->id])->update(['balance_cache' => $new_balance]);

        return $new_balance;
    }

}
